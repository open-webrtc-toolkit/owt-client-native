/*
 * Intel License
 */

#include "webrtc/base/base64.h"
#include "webrtc/base/criticalsection.h"
#include "webrtc/base/task_queue.h"
#include "talk/woogeen/sdk/conference/conferencepeerconnectionchannel.h"
#include "talk/woogeen/sdk/include/cpp/woogeen/conference/remotemixedstream.h"
#include "talk/woogeen/sdk/include/cpp/woogeen/conference/conferenceexception.h"
#include "talk/woogeen/sdk/include/cpp/woogeen/base/stream.h"
#include "talk/woogeen/sdk/include/cpp/woogeen/conference/conferenceclient.h"


namespace woogeen {
namespace conference {

enum ConferenceClient::StreamType : int {
  kStreamTypeCamera = 1,
  kStreamTypeScreen,
  kStreamTypeMix,
};

const std::string play_pause_failure_message =
    "Cannot play/pause a stream that have not been published or subscribed.";

std::shared_ptr<ConferenceClient> ConferenceClient::Create(
    const ConferenceClientConfiguration& configuration) {
  return std::shared_ptr<ConferenceClient>(new ConferenceClient(configuration));
}

ConferenceClient::ConferenceClient(
    const ConferenceClientConfiguration& configuration)
    : configuration_(configuration),
      event_queue_(new rtc::TaskQueue("ConferenceClientEventQueue")),
      signaling_channel_(new ConferenceSocketSignalingChannel()),
      signaling_channel_connected_(false) {}

ConferenceClient::~ConferenceClient() {
  signaling_channel_->RemoveObserver(*this);
}

void ConferenceClient::AddObserver(ConferenceClientObserver& observer) {
  const std::lock_guard<std::mutex> lock(observer_mutex_);
  std::vector<std::reference_wrapper<ConferenceClientObserver>>::iterator it =
      std::find_if(
          observers_.begin(), observers_.end(),
          [&](std::reference_wrapper<ConferenceClientObserver> o) -> bool {
      return &observer == &(o.get());
  });
  if (it != observers_.end()) {
      LOG(LS_INFO) << "Adding duplicate observer.";
      return;
  }
  observers_.push_back(observer);
}

void ConferenceClient::RemoveObserver(ConferenceClientObserver& observer) {
  const std::lock_guard<std::mutex> lock(observer_mutex_);
  observers_.erase(std::find_if(
      observers_.begin(), observers_.end(),
      [&](std::reference_wrapper<ConferenceClientObserver> o) -> bool {
        return &observer == &(o.get());
      }));
}

void ConferenceClient::Join(
    const std::string& token,
    std::function<void(std::shared_ptr<User>)> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (signaling_channel_connected_){
    if (on_failure != nullptr) {
      event_queue_->PostTask([on_failure]() {
        std::unique_ptr<ConferenceException> e(
            new ConferenceException(ConferenceException::kUnknown,
                                    "Already connected to conference server."));
        on_failure(std::move(e));
      });
    }
    return;
  }
  std::string token_base64(token);
  if (!rtc::Base64::IsBase64Encoded(token)) {
    LOG(LS_WARNING) << "Passing token with Base64 decoded is deprecated, "
                       "please pass it without modification.";
    token_base64 = rtc::Base64::Encode(token);
  }
  signaling_channel_->AddObserver(*this);
  signaling_channel_->Connect(token_base64, [=](sio::message::ptr info) {
    signaling_channel_connected_ = true;
    // Get current user's ID.
    std::string user_id;
    if (info->get_map()["id"]->get_flag() != sio::message::flag_string) {
      LOG(LS_ERROR) << "Room info doesn't contain client ID.";
      if (on_failure) {
        event_queue_->PostTask([on_failure]() {
          std::unique_ptr<ConferenceException> e(
              new ConferenceException(ConferenceException::kUnknown,
                                      "Received unknown message from MCU."));
          on_failure(std::move(e));
        });
      }
      return;
    } else {
      user_id = info->get_map()["id"]->get_string();
    }

    auto room_info = info->get_map()["room"];
    if (room_info == nullptr || room_info->get_flag() != sio::message::flag_object) {
        RTC_DCHECK(false);
        return;
    }
    // Trigger OnUserJoin for existed users.
    if (room_info->get_map()["participants"]->get_flag() != sio::message::flag_array) {
      LOG(LS_WARNING) << "Room info doesn't contain valid users.";
    } else {
      auto users = room_info->get_map()["participants"]->get_vector();
      // Get current user's ID and trigger |on_success|. Make sure |on_success|
      // is triggered before any other events because OnUserJoined and
      // OnStreamAdded should be triggered after join a conference.
      if (on_success) {
        auto user_it=users.begin();
        for (auto user_it = users.begin(); user_it != users.end(); user_it++) {
          if ((*user_it)->get_map()["id"]->get_string() == user_id) {
            User* user_raw;
            if (ParseUser(*user_it, &user_raw)) {
              std::shared_ptr<User> user(user_raw);
              event_queue_->PostTask([on_success, user]() { on_success(user); });
            } else if (on_failure) {
              event_queue_->PostTask([on_failure]() {
                std::unique_ptr<ConferenceException> e(new ConferenceException(
                    ConferenceException::kUnknown,
                    "Failed to parse current user's info"));
                on_failure(std::move(e));
              });
            }
            break;
          }
        }
        if (user_it == users.end()) {
          LOG(LS_ERROR) << "Cannot find current user's info in user list.";
          if (on_failure) {
            event_queue_->PostTask([on_failure]() {
              std::unique_ptr<ConferenceException> e(new ConferenceException(
                  ConferenceException::kUnknown,
                  "Received unknown message from MCU."));
              // TODO: Use async instead.
              on_failure(std::move(e));
            });
          }
        }
      }
      for (auto it = users.begin(); it != users.end(); ++it) {
        TriggerOnUserJoined(*it);
      }
    }
    // Trigger OnStreamAdded for existed remote streams.
    if (room_info->get_map()["streams"]->get_flag() != sio::message::flag_array) {
      LOG(LS_WARNING) << "Room info doesn't contain valid streams.";
    } else {
      auto streams = room_info->get_map()["streams"]->get_vector();
      for (auto it = streams.begin(); it != streams.end(); ++it) {
        LOG(LS_INFO) << "Find streams in the conference.";
        TriggerOnStreamAdded(*it);
      }
    }
  }, on_failure);
}

void ConferenceClient::Publish(
    std::shared_ptr<LocalStream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  PublishOptions options;
  Publish(stream, options, on_success, on_failure);
}

void ConferenceClient::Publish(
    std::shared_ptr<LocalStream> stream,
    const PublishOptions& options,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (!CheckNullPointer((uintptr_t)stream.get(), on_failure)) {
    LOG(LS_ERROR) << "Local stream cannot be nullptr.";
    return;
  }
  if (!CheckNullPointer((uintptr_t)stream.get(), on_failure)) {
    LOG(LS_ERROR) << "Cannot publish a local stream without media stream.";
    return;
  }
  {
    const std::lock_guard<std::mutex> lock(publish_pcs_mutex_);
    // TODO(jianlin): Remove publish-once restriction when we switch publish API
    // to return ConferenceLocalPublishSession.
    if (publish_pcs_.find(stream->MediaStream()->label()) !=
        publish_pcs_.end()) {
      LOG(LS_ERROR) << "Cannot publish a local stream to a specific conference "
                       "more than once.";
      if (on_failure) {
        event_queue_->PostTask([on_failure]() {
          std::unique_ptr<ConferenceException> e(new ConferenceException(
              ConferenceException::kUnknown, "Duplicated stream."));
          on_failure(std::move(e));
        });
      }
      return;
    }
  }
  if (!CheckSignalingChannelOnline(on_failure)) {
    return;
  }
  PeerConnectionChannelConfiguration config =
      GetPeerConnectionChannelConfiguration();
  if (options.max_audio_bandwidth > 0)
    config.max_audio_bandwidth = options.max_audio_bandwidth;
  if (options.max_video_bandwidth > 0)
    config.max_video_bandwidth = options.max_video_bandwidth;
  std::shared_ptr<ConferencePeerConnectionChannel> pcc(
      new ConferencePeerConnectionChannel(config, signaling_channel_,
                                          event_queue_));
  pcc->AddObserver(*this);
  {
    std::lock_guard<std::mutex> lock(publish_pcs_mutex_);
    publish_pcs_[stream->MediaStream()->label()] = pcc;
  }
  pcc->Publish(stream, on_success, on_failure);
}

void ConferenceClient::Subscribe(
    std::shared_ptr<RemoteStream> stream,
    std::function<void(std::shared_ptr<RemoteStream> stream)> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  SubscribeOptions options;
  Subscribe(stream, options, on_success, on_failure);
}

void ConferenceClient::Subscribe(
    std::shared_ptr<RemoteStream> stream,
    const SubscribeOptions& options,
    std::function<void(std::shared_ptr<RemoteStream> stream)> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (!CheckNullPointer((uintptr_t)stream.get(), on_failure)) {
    LOG(LS_ERROR) << "Local stream cannot be nullptr.";
    return;
  }
  if (!CheckSignalingChannelOnline(on_failure)) {
    return;
  }
  LOG(LS_INFO) << "Stream ID: "<<stream->Id();
  if (added_stream_type_.find(stream->Id()) == added_stream_type_.end()) {
    std::string failure_message(
        "Subscribing an invalid stream. Please check whether this stream is "
        "removed.");
    if (on_failure != nullptr) {
      event_queue_->PostTask([on_failure, failure_message]() {
        std::unique_ptr<ConferenceException> e(new ConferenceException(
            ConferenceException::kUnknown, failure_message));
        on_failure(std::move(e));
      });
    }
    return;
  }
  {
    std::lock_guard<std::mutex> lock(subscribe_pcs_mutex_);
    if (subscribe_pcs_.find(stream->Id()) != subscribe_pcs_.end()) {
      if (on_failure != nullptr) {
        event_queue_->PostTask([on_failure]() {
          std::string failure_message(
              "Cannot subscribe a stream that is subscribing.");
          std::unique_ptr<ConferenceException> e(new ConferenceException(
              ConferenceException::kUnknown, failure_message));
          on_failure(std::move(e));
        });
      }
      return;
    }
  }
  PeerConnectionChannelConfiguration config =
      GetPeerConnectionChannelConfiguration();
  std::shared_ptr<ConferencePeerConnectionChannel> pcc(
      new ConferencePeerConnectionChannel(config, signaling_channel_,
                                          event_queue_));
  pcc->AddObserver(*this);
  {
    std::lock_guard<std::mutex> lock(subscribe_pcs_mutex_);
    subscribe_pcs_[stream->Id()] = pcc;
  }
  if (added_stream_type_[stream->Id()] == kStreamTypeMix) {
    pcc->Subscribe(std::static_pointer_cast<RemoteMixedStream>(stream), options,
                   on_success, on_failure);
  } else if (added_stream_type_[stream->Id()] == kStreamTypeScreen) {
    pcc->Subscribe(std::static_pointer_cast<RemoteScreenStream>(stream),
                   options, on_success, on_failure);
  } else if (added_stream_type_[stream->Id()] == kStreamTypeCamera) {
    pcc->Subscribe(std::static_pointer_cast<RemoteCameraStream>(stream),
                   options, on_success, on_failure);
  }
}

void ConferenceClient::Unpublish(
    std::shared_ptr<LocalStream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (!CheckNullPointer((uintptr_t)stream.get(), on_failure)) {
    LOG(LS_ERROR) << "Local stream cannot be nullptr.";
    return;
  }
  if (!CheckNullPointer((uintptr_t)stream->MediaStream(), on_failure)) {
    LOG(LS_ERROR) << "Cannot publish a local stream without media stream.";
    return;
  }
  if (!CheckSignalingChannelOnline(on_failure)) {
    return;
  }
  {
    std::lock_guard<std::mutex> lock(publish_pcs_mutex_);
    auto pcc_it = publish_pcs_.find(stream->MediaStream()->label());
    if (pcc_it == publish_pcs_.end()) {
      LOG(LS_ERROR) << "Cannot find peerconnection channel for stream.";
      if (on_failure != nullptr) {
        event_queue_->PostTask([on_failure]() {
          std::unique_ptr<ConferenceException> e(new ConferenceException(
              ConferenceException::kUnknown, "Invalid stream."));
          on_failure(std::move(e));
        });
      }
    } else {
      pcc_it->second->Unpublish(
          stream,
          [=]() {
            if (on_success != nullptr)
              event_queue_->PostTask([on_success]() { on_success(); });
            {
              std::lock_guard<std::mutex> lock(publish_pcs_mutex_);
              publish_pcs_.erase(pcc_it);
            }
          },
          on_failure);
    }
  }
}

void ConferenceClient::Unsubscribe(
    std::shared_ptr<RemoteStream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (!CheckNullPointer((uintptr_t)stream.get(), on_failure)) {
    LOG(LS_ERROR) << "Remote stream cannot be nullptr.";
    return;
  }
  if (!CheckSignalingChannelOnline(on_failure)) {
    return;
  }
  LOG(LS_INFO) << "About to unsubscribe stream " << stream->Id();
  {
    std::lock_guard<std::mutex> lock(subscribe_pcs_mutex_);
    std::string id;

    auto label_it = subscribe_id_label_map_.find(stream->Id());
    if (label_it != subscribe_id_label_map_.end()) {
        id = label_it->second;
    }

    auto pcc_it = subscribe_pcs_.find(stream->Id());
    if (pcc_it == subscribe_pcs_.end()) {
      LOG(LS_ERROR) << "Cannot find peerconnection channel for stream.";
      if (on_failure != nullptr) {
        event_queue_->PostTask([on_failure]() {
          std::unique_ptr<ConferenceException> e(new ConferenceException(
              ConferenceException::kUnknown, "Invalid stream."));
          on_failure(std::move(e));
        });
      }
    } else {
      pcc_it->second->Unsubscribe(
          stream,
          [=]() {
            if (on_success != nullptr)
              event_queue_->PostTask([on_success]() { on_success(); });
            {
              std::lock_guard<std::mutex> lock(subscribe_pcs_mutex_);
              subscribe_pcs_.erase(pcc_it);
              subscribe_id_label_map_.erase(id);
            }
          },
          on_failure);
    }
  }
}

void ConferenceClient::Send(
    const std::string& message,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (!CheckSignalingChannelOnline(on_failure)) {
    return;
  }
  std::string receiver("");
  Send(message, receiver, on_success, on_failure);
}

void ConferenceClient::Send(
    const std::string& message,
    const std::string& receiver,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (!CheckSignalingChannelOnline(on_failure)) {
    return;
  }
  signaling_channel_->SendCustomMessage(
      message, receiver, RunInEventQueue(on_success), on_failure);
}

void ConferenceClient::PlayAudio(
    std::shared_ptr<LocalStream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (!CheckSignalingChannelOnline(on_failure)) {
    return;
  }
  auto pc = GetConferencePeerConnectionChannel(stream);
  if (!CheckNullPointer((uintptr_t)pc.get(), play_pause_failure_message,
                        on_failure)) {
    return;
  }
  pc->PlayAudio(stream, on_success, on_failure);
}

void ConferenceClient::PauseAudio(
    std::shared_ptr<LocalStream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  auto pc = GetConferencePeerConnectionChannel(stream);
  if (!CheckNullPointer((uintptr_t)pc.get(), play_pause_failure_message,
                        on_failure)) {
    return;
  }
  if (!CheckSignalingChannelOnline(on_failure)) {
    return;
  }
  pc->PauseAudio(stream, on_success, on_failure);
}

void ConferenceClient::PlayVideo(
    std::shared_ptr<LocalStream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  auto pc = GetConferencePeerConnectionChannel(stream);
  if (!CheckNullPointer((uintptr_t)pc.get(), play_pause_failure_message,
                        on_failure)) {
    return;
  }
  if (!CheckSignalingChannelOnline(on_failure)) {
    return;
  }
  pc->PlayVideo(stream, on_success, on_failure);
}

void ConferenceClient::PauseVideo(
    std::shared_ptr<LocalStream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  auto pc = GetConferencePeerConnectionChannel(stream);
  if (!CheckNullPointer((uintptr_t)pc.get(), play_pause_failure_message,
                        on_failure)) {
    return;
  }
  if (!CheckSignalingChannelOnline(on_failure)) {
    return;
  }
  pc->PauseVideo(stream, on_success, on_failure);
}

void ConferenceClient::PlayAudio(
    std::shared_ptr<RemoteStream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
    if (!CheckSignalingChannelOnline(on_failure)) {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(subscribe_pcs_mutex_);
        std::string id;

        auto label_it = subscribe_id_label_map_.find(stream->Id());
        if (label_it != subscribe_id_label_map_.end()) {
            id = label_it->second;
        }

        auto pcc_it = subscribe_pcs_.find(id);
        if (pcc_it == subscribe_pcs_.end()) {
            LOG(LS_ERROR) << "Cannot find peerconnection channel for stream.";
            if (on_failure != nullptr) {
                event_queue_->PostTask([on_failure]() {
                    std::unique_ptr<ConferenceException> e(new ConferenceException(
                        ConferenceException::kUnknown, "Invalid stream."));
                    on_failure(std::move(e));
                });
            }
        }
        else {
            pcc_it->second->PlayAudio(stream, on_success, on_failure);
        }
    };
}

void ConferenceClient::PauseAudio(
    std::shared_ptr<RemoteStream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
    if (!CheckSignalingChannelOnline(on_failure)) {
      return;
    }
    {
      std::lock_guard<std::mutex> lock(subscribe_pcs_mutex_);
      std::string id;

      auto label_it = subscribe_id_label_map_.find(stream->Id());
      if (label_it != subscribe_id_label_map_.end()) {
        id = label_it->second;
      }

      auto pcc_it = subscribe_pcs_.find(id);
      if (pcc_it == subscribe_pcs_.end()) {
        LOG(LS_ERROR) << "Cannot find peerconnection channel for stream.";
        if (on_failure != nullptr) {
          event_queue_->PostTask([on_failure]() {
                    std::unique_ptr<ConferenceException> e(new ConferenceException(
                        ConferenceException::kUnknown, "Invalid stream."));
                    on_failure(std::move(e));
          });
        }
      } else {
        pcc_it->second->PauseAudio(stream, on_success, on_failure);
      }
    }
}

void ConferenceClient::PlayVideo(
    std::shared_ptr<RemoteStream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (!CheckSignalingChannelOnline(on_failure)) {
    return;
  }
  {
    std::lock_guard<std::mutex> lock(subscribe_pcs_mutex_);
    std::string id;

    auto label_it = subscribe_id_label_map_.find(stream->Id());
    if (label_it != subscribe_id_label_map_.end()) {
      id = label_it->second;
    }

    auto pcc_it = subscribe_pcs_.find(id);
    if (pcc_it == subscribe_pcs_.end()) {
      LOG(LS_ERROR) << "Cannot find peerconnection channel for stream.";
      if (on_failure != nullptr) {
          event_queue_->PostTask([on_failure]() {
                  std::unique_ptr<ConferenceException> e(new ConferenceException(
                      ConferenceException::kUnknown, "Invalid stream."));
                  on_failure(std::move(e));
          });
       }
    } else {
      pcc_it->second->PlayVideo(stream, on_success, on_failure);
    }
  }
}

void ConferenceClient::PauseVideo(
    std::shared_ptr<RemoteStream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
    if (!CheckSignalingChannelOnline(on_failure)) {
      return;
    }
    {
      std::lock_guard<std::mutex> lock(subscribe_pcs_mutex_);
      std::string id;

      auto label_it = subscribe_id_label_map_.find(stream->Id());
      if (label_it != subscribe_id_label_map_.end()) {
        id = label_it->second;
      }

      auto pcc_it = subscribe_pcs_.find(id);
      if (pcc_it == subscribe_pcs_.end()) {
        LOG(LS_ERROR) << "Cannot find peerconnection channel for stream.";
        if (on_failure != nullptr) {
           event_queue_->PostTask([on_failure]() {
                        std::unique_ptr<ConferenceException> e(new ConferenceException(
                            ConferenceException::kUnknown, "Invalid stream."));
                        on_failure(std::move(e));
                    });
         }
      } else {
        pcc_it->second->PauseVideo(stream, on_success, on_failure);
      }
    }
}

void ConferenceClient::Leave(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (!CheckSignalingChannelOnline(on_failure)) {
    return;
  }
  {
    std::lock_guard<std::mutex> lock(publish_pcs_mutex_);
    publish_id_label_map_.clear();
    publish_pcs_.clear();
  }
  {
    std::lock_guard<std::mutex> lock(subscribe_pcs_mutex_);
    subscribe_pcs_.clear();
  }
  signaling_channel_->Disconnect(RunInEventQueue(on_success), on_failure);
}

void ConferenceClient::GetRegion(
      std::shared_ptr<RemoteStream> stream,
      std::shared_ptr<RemoteMixedStream> mixed_stream,
      std::function<void(std::string)> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure){
  if (!CheckNullPointer((uintptr_t)stream.get(), on_failure) ||
      !CheckNullPointer((uintptr_t)mixed_stream.get(), on_failure)) {
    return;
  }
  if (!CheckSignalingChannelOnline(on_failure)){
    return;
  }
  // If current stream is already subscribed, we use the label as stream ID;
  // otherwise use the stream id that's not yet modified.
  std::string id;
  {
    std::lock_guard<std::mutex> lock(subscribe_pcs_mutex_);
    // Search subscribe pcs.
    auto label_it = subscribe_id_label_map_.find(stream->Id());
    if (label_it != subscribe_id_label_map_.end()) {
      id = label_it->second;
    } else {
      id = stream->Id();
    }
  }
  signaling_channel_->GetRegion(id, mixed_stream->Viewport(), on_success,
        on_failure);
  return;
}

void ConferenceClient::SetRegion(
    std::shared_ptr<RemoteStream> stream,
    std::shared_ptr<RemoteMixedStream> mixed_stream,
    const std::string& region_id,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (!CheckNullPointer((uintptr_t)stream.get(), on_failure) ||
      !CheckNullPointer((uintptr_t)mixed_stream.get(), on_failure)) {
    return;
  }
  if (!CheckSignalingChannelOnline(on_failure)) {
    return;
  }
  // If current stream is already subscribed, we use the label as stream ID;
  // otherwise use the stream id that's not yet modified.
  std::string id;
  {
    std::lock_guard<std::mutex> lock(subscribe_pcs_mutex_);
    // Search subscribe pcs.
    auto label_it = subscribe_id_label_map_.find(stream->Id());
    if (label_it != subscribe_id_label_map_.end()) {
      id = label_it->second;
    } else {
      id = stream->Id();
    }
  }
  signaling_channel_->SetRegion(id, mixed_stream->Viewport(), region_id,
                                RunInEventQueue(on_success), on_failure);
}

void ConferenceClient::Mute(
    std::shared_ptr<Stream> stream,
    bool mute_audio,
    bool mute_video,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (!CheckNullPointer((uintptr_t)stream.get(), on_failure)) {
    return;
  }
  if (!CheckSignalingChannelOnline(on_failure)) {
    return;
  }
  std::string id;
  {
    std::lock_guard<std::mutex> lock(subscribe_pcs_mutex_);
    // Search subscribe pcs.
    auto label_it = subscribe_id_label_map_.find(stream->Id());
    if (label_it != subscribe_id_label_map_.end()) {
      id = label_it->second;
    } else {
      id = stream->Id();
    }
  }
  signaling_channel_->Mute(id, mute_audio, mute_video,
                           RunInEventQueue(on_success), on_failure);
}

void ConferenceClient::Unmute(
    std::shared_ptr<Stream> stream,
    bool unmute_audio,
    bool unmute_video,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (!CheckNullPointer((uintptr_t)stream.get(), on_failure)) {
    return;
  }
  if (!CheckSignalingChannelOnline(on_failure)) {
    return;
  }
  std::string id;
  {
    std::lock_guard<std::mutex> lock(subscribe_pcs_mutex_);
    // Search subscribe pcs.
    auto label_it = subscribe_id_label_map_.find(stream->Id());
    if (label_it != subscribe_id_label_map_.end()) {
      id = label_it->second;
    } else {
      id = stream->Id();
    }
  }
  signaling_channel_->Unmute(id, unmute_audio, unmute_video,
                             RunInEventQueue(on_success), on_failure);
}

void ConferenceClient::Mix(
    std::shared_ptr<Stream> stream,
    std::vector<std::shared_ptr<RemoteMixedStream>> mixed_stream_list,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (!CheckNullPointer((uintptr_t)stream.get(), on_failure)) {
    return;
  }
  if (!CheckSignalingChannelOnline(on_failure)) {
    return;
  }
  std::vector<std::string> mixed_stream_ids;
  for (auto mixed_stream : mixed_stream_list) {
    if (mixed_stream.get())
      mixed_stream_ids.push_back(mixed_stream->Viewport());
  }
  if (mixed_stream_ids.empty() && on_failure) {
    event_queue_->PostTask([on_failure]() {
      std::unique_ptr<ConferenceException> e(new ConferenceException(
          ConferenceException::kUnknown,
          "No valid remote mixed stream specified for the mix request."));
      on_failure(std::move(e));
    });
    return;
  }
  std::string id;
  {
    std::lock_guard<std::mutex> lock(subscribe_pcs_mutex_);
    // Search subscribe pcs.
    auto label_it = subscribe_id_label_map_.find(stream->Id());
    if (label_it != subscribe_id_label_map_.end()) {
      id = label_it->second;
    } else {
      id = stream->Id();
    }
  }
  signaling_channel_->Mix(id, mixed_stream_ids,
                          RunInEventQueue(on_success), on_failure);
}

void ConferenceClient::Unmix(
    std::shared_ptr<Stream> stream,
    std::vector<std::shared_ptr<RemoteMixedStream>> mixed_stream_list,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (!CheckNullPointer((uintptr_t)stream.get(), on_failure)) {
    return;
  }
  if (!CheckSignalingChannelOnline(on_failure)) {
    return;
  }
  std::vector<std::string> mixed_stream_ids;
  for (auto mixed_stream : mixed_stream_list) {
    if (mixed_stream.get()) {
      mixed_stream_ids.push_back(mixed_stream->Viewport());
    }
  }
  if (mixed_stream_ids.empty() && on_failure) {
    event_queue_->PostTask([on_failure]() {
      std::unique_ptr<ConferenceException> e(new ConferenceException(
          ConferenceException::kUnknown,
          "No valid remote mixed stream specified for the unmix request."));
      on_failure(std::move(e));
    });
    return;
  }
  std::string id;
  {
    std::lock_guard<std::mutex> lock(subscribe_pcs_mutex_);
    // Search subscribe pcs.
    auto label_it = subscribe_id_label_map_.find(stream->Id());
    if (label_it != subscribe_id_label_map_.end()) {
      id = label_it->second;
    } else {
      id = stream->Id();
    }
  }

  signaling_channel_->Unmix(id, mixed_stream_ids,
                            RunInEventQueue(on_success), on_failure);
}

void ConferenceClient::AddExternalOutput(
    const ExternalOutputOptions& options,
    std::function<void(std::shared_ptr<ExternalOutputAck>)> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (!CheckNullPointer((uintptr_t)options.stream.get(), on_failure)) {
    return;
  }
  if (!CheckSignalingChannelOnline(on_failure)) {
    return;
  }
  if (options.url != "file") {
    signaling_channel_->AddOrUpdateExternalOutput(true, options, on_success,
                                                  on_failure);
  } else {
    signaling_channel_->AddOrUpdateRecorder(true, options, on_success,
                                            on_failure);
  }
}

void ConferenceClient::UpdateExternalOutput(
    const ExternalOutputOptions& options,
    std::function<void(std::shared_ptr<ExternalOutputAck>)> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (!CheckNullPointer((uintptr_t)options.stream.get(), on_failure)) {
    return;
  }
  if (!CheckSignalingChannelOnline(on_failure)) {
    return;
  }
  if (options.url != "file") {
    signaling_channel_->AddOrUpdateExternalOutput(false, options, on_success,
                                                  on_failure);
  } else {
    RTC_NOTREACHED() << "Update recorder is not supported.";
  }
}

void ConferenceClient::RemoveExternalOutput(
    const std::string& url,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (!CheckSignalingChannelOnline(on_failure)) {
    return;
  }
  std::string rtsp("rtsp://");
  std::string rtmp("rtmp://");
  if (url.compare(0, rtsp.size(), rtsp) || url.compare(0, rtmp.size(), rtmp)) {
    signaling_channel_->RemoveRecorder(url, on_success, on_failure);
  } else {
    signaling_channel_->RemoveRecorder(url, on_success, on_failure);
  }
}

void ConferenceClient::GetConnectionStats(
    std::shared_ptr<Stream> stream,
    std::function<void(std::shared_ptr<ConnectionStats>)> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (!CheckNullPointer((uintptr_t)stream.get(), on_failure)) {
    return;
  }
  auto pcc = GetConferencePeerConnectionChannel(stream->Id());
  if (pcc == nullptr) {
    if (on_failure) {
      event_queue_->PostTask([on_failure]() {
        std::unique_ptr<ConferenceException> e(
            new ConferenceException(ConferenceException::kUnknown,
                                    "Stream is not published or subscribed."));

        on_failure(std::move(e));
      });
    }
    LOG(LS_WARNING)
        << "Tried to get connection statistics from unknown stream.";
    return;
  }
  pcc->GetConnectionStats(stream, on_success, on_failure);
}

void ConferenceClient::OnStreamAdded(sio::message::ptr stream) {
  TriggerOnStreamAdded(stream);
}

void ConferenceClient::OnCustomMessage(std::string& from,
                                       std::string& message) {
  LOG(LS_INFO) << "ConferenceClient OnCustomMessage";
  for (auto its = observers_.begin(); its != observers_.end(); ++its) {
    (*its).get().OnMessageReceived(from, message);
  }
}

void ConferenceClient::OnSignalingMessage(sio::message::ptr message) {
  // MCU returns inconsistent format for this event. :(
  std::string stream_id = (message->get_map()["peerId"] == nullptr)
                              ? message->get_map()["id"]->get_string()
                              : message->get_map()["peerId"]->get_string();
  auto pcc = GetConferencePeerConnectionChannel(stream_id);
  if (pcc == nullptr) {
    LOG(LS_WARNING) << "Received signaling message from unknown sender.";
    return;
  }
  // Check the status before delivering to pcc.
  auto soac_status = message->get_map()["status"];
  if (soac_status == nullptr || soac_status->get_flag() != sio::message::flag_string
      || (soac_status->get_string() != "soac" && soac_status->get_string() != "ready"
      && soac_status->get_string() != "error")) {
    LOG(LS_INFO) << "Ignore signaling status except soac/ready/error";
    return;
  }

  if (soac_status->get_string() == "ready") {
    sio::message::ptr success_msg = sio::string_message::create("success");
    pcc->OnSignalingMessage(success_msg);
    return;
  }
  else if (soac_status->get_string() == "error") {
    sio::message::ptr failure_msg = sio::string_message::create("failure");
    pcc->OnSignalingMessage(failure_msg);
    return;
  }
  auto soac_data = message->get_map()["data"];
  if (soac_data == nullptr || soac_data->get_flag() != sio::message::flag_object) {
    LOG(LS_ERROR) << "Received siganling message without offer, answer or candidate";
    return;
  }

  pcc->OnSignalingMessage(message->get_map()["data"]);
}

void ConferenceClient::OnStreamRemoved(sio::message::ptr stream) {
  TriggerOnStreamRemoved(stream);
}

void ConferenceClient::OnStreamUpdated(sio::message::ptr stream) {
  TriggerOnStreamUpdated(stream);
}

void ConferenceClient::OnStreamError(sio::message::ptr stream) {
  if (stream == nullptr || stream->get_flag() != sio::message::flag_object ||
      stream->get_map()["streamId"] == nullptr ||
      stream->get_map()["streamId"]->get_flag() != sio::message::flag_string) {
    RTC_DCHECK(false);
    return;
  }
  const std::string stream_id(stream->get_map()["streamId"]->get_string());
  LOG(LS_ERROR) << "MCU reports connection failed for stream " << stream_id;
  auto pcc = GetConferencePeerConnectionChannel(stream_id);
  if (pcc == nullptr) {
    RTC_DCHECK(false);
    return;
  }
  pcc->OnStreamError(
      std::string("MCU reported an error was occurred for certain stream."));
}

void ConferenceClient::OnServerDisconnected() {
  signaling_channel_connected_ = false;
  {
    std::lock_guard<std::mutex> lock(publish_pcs_mutex_);
    publish_id_label_map_.clear();
    publish_pcs_.clear();
  }
  {
    std::lock_guard<std::mutex> lock(subscribe_pcs_mutex_);
    subscribe_pcs_.clear();
    subscribe_id_label_map_.clear();
  }
  for (auto its = observers_.begin(); its != observers_.end(); ++its) {
    (*its).get().OnServerDisconnected();
  }
}

void ConferenceClient::OnStreamError(
    std::shared_ptr<Stream> stream,
    std::shared_ptr<const ConferenceException> exception) {
  TriggerOnStreamError(stream, exception);
}

void ConferenceClient::OnStreamId(const std::string& id,
                                  const std::string& publish_stream_label) {
  {
    std::lock_guard<std::mutex> lock(publish_pcs_mutex_);
    publish_id_label_map_[id] = publish_stream_label;
  }
  auto pcc = GetConferencePeerConnectionChannel(id);
  RTC_CHECK(pcc != nullptr);
  pcc->SetStreamId(id);
}

void ConferenceClient::OnSubscriptionId(const std::string& subscription_id,
                                        const std::string& stream_id) {
  {
    std::lock_guard<std::mutex> lock(subscribe_pcs_mutex_);
    subscribe_id_label_map_[subscription_id] = stream_id;
  }
  auto pcc = GetConferencePeerConnectionChannel(stream_id);
  RTC_CHECK(pcc != nullptr);
  pcc->SetStreamId(subscription_id);
}

bool ConferenceClient::CheckNullPointer(
    uintptr_t pointer,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  std::string failure_message="Null pointer is not allowed.";
  return CheckNullPointer(pointer, failure_message, on_failure);
}

bool ConferenceClient::CheckNullPointer(
    uintptr_t pointer,
    const std::string& failure_message,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (pointer)
    return true;
  if (on_failure != nullptr) {
    event_queue_->PostTask([on_failure, failure_message]() {
      std::unique_ptr<ConferenceException> e(new ConferenceException(
          ConferenceException::kUnknown, failure_message));
      on_failure(std::move(e));
    });
  }
  return false;
}

bool ConferenceClient::CheckSignalingChannelOnline(
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure){
  if (signaling_channel_connected_)
    return true;
  if (on_failure != nullptr) {
    event_queue_->PostTask([on_failure]() {
      std::unique_ptr<ConferenceException> e(
          new ConferenceException(ConferenceException::kUnknown,
                                  "Conference server is not connected."));
      on_failure(std::move(e));
    });
  }
  return false;
}


void ConferenceClient::TriggerOnStreamAdded(sio::message::ptr stream_info) {
  std::string id = stream_info->get_map()["id"]->get_string();
#if 0
  auto data = stream_info->get_map()["data"];
  if (data == nullptr || data->get_flag() != sio::message::flag_object) {
    RTC_DCHECK(false);
    LOG(LS_ERROR) << "Invalid data from stream " << id
                  << "is invalid, this stream will be ignored.";
    return;
  }
#endif
  auto media_info = stream_info->get_map()["media"];
  if (media_info == nullptr || media_info->get_flag() != sio::message::flag_object) {
    RTC_DCHECK(false);
    LOG(LS_ERROR) << "Invalid media info from stream " << id
                  << ", this stream will be ignored.";
    return;
  }
  bool has_audio = false;
  auto audio_info = media_info->get_map()["audio"];
  if (audio_info == nullptr) {
    LOG(LS_INFO) << "No audio in stream " << id;
  } else {
    has_audio = true;
  }

  std::vector<VideoFormat> video_formats;
  bool has_video = false;
  bool is_screencast = false;
  auto video_info = media_info->get_map()["video"];
  if (video_info == nullptr) {
    LOG(LS_INFO) << "No video in stream " << id;
  } else {
    has_video = true;
    // Check if underlying video stream source is screen-cast.
    // Remote stream from file and camera is handle the same way.
    if (video_info->get_flag() == sio::message::flag_object) {
      auto video_source = video_info->get_map()["source"];

      if (video_source != nullptr && video_source->get_flag() == sio::message::flag_string
          && video_source->get_string() == "screen-cast") {
         is_screencast = true;
      }
      // First check the main supported resolution. At present video format for
      // forward stream only differs in resolution, not codec or bitrate.
      auto video_params = video_info->get_map()["parameters"];
      if (video_params != nullptr && video_params->get_flag() == sio::message::flag_object) {
        auto main_resolution = video_params->get_map()["resolution"];
        if (main_resolution != nullptr && main_resolution->get_flag() == sio::message::flag_object) {
          Resolution resolution = Resolution(main_resolution->get_map()["width"]->get_int(),
                                             main_resolution->get_map()["height"]->get_int());
          const VideoFormat video_format(resolution);
          video_formats.push_back(video_format);
        }
      }
      // Add optional supported resolutions.
      auto video_optional_params = video_info->get_map()["optional"];
      if (video_optional_params != nullptr && video_optional_params->get_flag() == sio::message::flag_object) {
         auto optional_parameters = video_optional_params->get_map()["parameters"];
         if (optional_parameters != nullptr && optional_parameters->get_flag() == sio::message::flag_object
             && optional_parameters->get_map()["resolution"]) {
           auto resolutions = optional_parameters->get_map()["resolution"]->get_vector();
             for (auto it = resolutions.begin(); it != resolutions.end(); ++it) {
               Resolution resolution((*it)->get_map()["width"]->get_int(),
                                (*it)->get_map()["height"]->get_int());
               const VideoFormat video_format(resolution);
               video_formats.push_back(video_format);
             }
         }
      }
    }
  }

  // TODO(jianlin): handle the bitrate and format(codec/profile) information
  auto stream_type = stream_info->get_map()["type"];
  if (stream_type == nullptr || stream_type->get_flag() != sio::message::flag_string) {
    RTC_DCHECK(false);
    LOG(LS_ERROR) << "Invalid stream type from stream " << id
                   << ", this stream will be ignored";
    return;
  }
  if (stream_info->get_map()["type"]->get_string() == "forward") { // forward stream. PublicationInfo provided.
    auto pub_info = stream_info->get_map()["info"];
    if (pub_info == nullptr || pub_info->get_flag() != sio::message::flag_object) {
      RTC_DCHECK(false);
      LOG(LS_ERROR) << "Invalid publication info from stream " << id
                   << ", this stream will be ignored";
      return;
    }
    // remote_id here stands for participantID
    std::string remote_id = pub_info->get_map()["owner"]->get_string();
    std::unordered_map<std::string, std::string> attributes =
         AttributesFromStreamInfo(pub_info);
    // TODO: use the underlying stream's label as the ID instead of using
    // the publiciationID.
    if (!is_screencast) {
      auto remote_stream = std::make_shared<RemoteCameraStream>(id, remote_id);
      remote_stream->has_audio_ = has_audio;
      remote_stream->has_video_ = has_video;
      remote_stream->Attributes(attributes);
      added_streams_[id] = remote_stream;
      added_stream_type_[id] = StreamType::kStreamTypeCamera;
      for (auto its = observers_.begin(); its != observers_.end(); ++its) {
        auto& o = (*its).get();
        event_queue_->PostTask(
            [&o, remote_stream] { o.OnStreamAdded(remote_stream); });
      }
    } else {
      auto remote_stream = std::make_shared<RemoteScreenStream>(id, remote_id);
      LOG(LS_INFO) << "OnStreamAdded: screen stream.";
      remote_stream->has_audio_ = has_audio;
      remote_stream->has_video_ = true;
      remote_stream->Attributes(attributes);
      added_streams_[id] = remote_stream;
      added_stream_type_[id] = StreamType::kStreamTypeScreen;
      for (auto its = observers_.begin(); its != observers_.end(); ++its) {
        auto& o = (*its).get();
        event_queue_->PostTask(
            [&o, remote_stream] { o.OnStreamAdded(remote_stream); });
      }
    }
  } else if(stream_info->get_map()["type"]->get_string() == "mixed") { // mixed stream. ViewInfo provided.
    auto view_info = stream_info->get_map()["info"];
    if (view_info == nullptr || view_info->get_flag() != sio::message::flag_object) {
      RTC_DCHECK(false);
      LOG(LS_ERROR) << "Invalid view info from stream " << id
                   << ", this stream will be ignored";
      return;
    }
    std::string viewport("");
    if (view_info->get_map()["label"] && view_info->get_map()["label"]->get_flag() ==
        sio::message::flag_string) {
      viewport = view_info->get_map()["label"]->get_string();
    }
    std::string remote_id("mcu"); // Not used.
    auto remote_stream = std::make_shared<RemoteMixedStream>(
          id, remote_id, viewport, video_formats);
      LOG(LS_INFO) << "OnStreamAdded: mixed stream.";
      remote_stream->has_audio_ = has_audio;
      remote_stream->has_video_ = has_video;
      added_streams_[id] = remote_stream;
      added_stream_type_[id] = StreamType::kStreamTypeMix;
      for (auto its = observers_.begin(); its != observers_.end(); ++its) {
        auto& o = (*its).get();
        event_queue_->PostTask(
            [&o, remote_stream] { o.OnStreamAdded(remote_stream); });
      }
  } else {
    RTC_DCHECK(false);
    LOG(LS_ERROR) << "Invalid stream type from stream " << id
                   << ", this stream will be ignored";
    return;
  }

}

void ConferenceClient::TriggerOnUserJoined(sio::message::ptr user_info) {
  User* user_raw;
  if(ParseUser(user_info, &user_raw)){
    std::shared_ptr<User> user(user_raw);
    participants[user->Id()] = user;
    const std::lock_guard<std::mutex> lock(observer_mutex_);
    for (auto its = observers_.begin(); its != observers_.end(); ++its) {
      auto& o=(*its).get();
      event_queue_->PostTask([&o, user] { o.OnUserJoined(user); });
    }
  }
}

void ConferenceClient::TriggerOnUserLeft(sio::message::ptr user_info) {
  if (user_info == nullptr ||
      user_info->get_flag() != sio::message::flag_string) {
    RTC_DCHECK(false);
    return;
  }
  auto user_id = user_info->get_string();
  auto user_it = participants.find(user_id);
  if (user_it == participants.end()) {
    RTC_DCHECK(false);
    return;
  }
  auto user = user_it->second;
  participants.erase(user_it);
  const std::lock_guard<std::mutex> lock(observer_mutex_);
  for (auto its = observers_.begin(); its != observers_.end(); ++its) {
    auto& o = (*its).get();
    event_queue_->PostTask([&o, user] { o.OnUserLeft(user); });
  }
}

bool ConferenceClient::ParseUser(sio::message::ptr user_message,
                                 User** user) const {
  if (user_message == nullptr ||
      user_message->get_flag() != sio::message::flag_object ||
      user_message->get_map()["id"] == nullptr ||
      user_message->get_map()["id"]->get_flag() != sio::message::flag_string ||
      user_message->get_map()["user"] == nullptr ||
      user_message->get_map()["user"]->get_flag() !=
          sio::message::flag_string ||
      user_message->get_map()["role"] == nullptr ||
      user_message->get_map()["role"]->get_flag() !=
          sio::message::flag_string) {
    RTC_DCHECK(false);
    return false;
  }
  std::string id = user_message->get_map()["id"]->get_string();
  std::string user_name = user_message->get_map()["user"]->get_string();
  std::string role = user_message->get_map()["role"]->get_string();
  // TODO(jianjunz): Parse permission info when server side better designed
  // permission structure.
  bool publish = true;
  bool subscribe = true;
  bool record = true;
  conference::Permission permission(publish, subscribe, record);

  *user = new User(id, user_name, role, permission);
  return true;
}

std::shared_ptr<ConferencePeerConnectionChannel>
ConferenceClient::GetConferencePeerConnectionChannel(
    std::shared_ptr<LocalStream> stream) const {
  if (stream == nullptr) {
    LOG(LS_WARNING) << "Cannot get PeerConnectionChannel for a null stream.";
    return nullptr;
  }
  if (stream->MediaStream() == nullptr) {
    LOG(LS_WARNING) << "Cannot find publish PeerConnectionChannel for a stream "
                     "without media stream.";
    return nullptr;
  }
  {
    std::lock_guard<std::mutex> lock(publish_pcs_mutex_);
    auto pcc_it = publish_pcs_.find(stream->MediaStream()->label());
    if (pcc_it != publish_pcs_.end()) {
      return pcc_it->second;
    }
  }
  return nullptr;
}

std::shared_ptr<ConferencePeerConnectionChannel>
ConferenceClient::GetConferencePeerConnectionChannel(
    std::shared_ptr<RemoteStream> stream) const {
  if (stream == nullptr) {
    LOG(LS_ERROR) << "Cannot get PeerConnectionChannel for a null stream.";
    return nullptr;
  }
  {
    std::lock_guard<std::mutex> lock(subscribe_pcs_mutex_);
    auto pcc_it = subscribe_pcs_.find(stream->Id());
    if (pcc_it != subscribe_pcs_.end()) {
      return pcc_it->second;
    }
  }
  return nullptr;
}

std::shared_ptr<ConferencePeerConnectionChannel>
ConferenceClient::GetConferencePeerConnectionChannel(
    std::shared_ptr<Stream> stream) const {
  if (stream == nullptr) {
    LOG(LS_ERROR) << "Cannot get PeerConnectionChannel for a null stream.";
    return nullptr;
  }
  {
    std::lock_guard<std::mutex> lock(subscribe_pcs_mutex_);
    auto pcc_it = subscribe_pcs_.find(stream->Id());
    if (pcc_it != subscribe_pcs_.end()) {
      return pcc_it->second;
    }
  }
  if (stream->MediaStream() == nullptr) {
    LOG(LS_ERROR) << "Cannot find publish PeerConnectionChannel for a stream "
                     "without media stream.";
    return nullptr;
  }
  {
    std::lock_guard<std::mutex> lock(publish_pcs_mutex_);
    auto pcc_it = publish_pcs_.find(stream->MediaStream()->label());
    if (pcc_it != publish_pcs_.end()) {
      return pcc_it->second;
    }
    LOG(LS_ERROR) << "Cannot find PeerConnectionChannel for specific stream.";
    return nullptr;
  }
}

std::shared_ptr<ConferencePeerConnectionChannel>
ConferenceClient::GetConferencePeerConnectionChannel(
    const std::string& stream_id) const {
  std::string id;
  {
    std::lock_guard<std::mutex> lock(subscribe_pcs_mutex_);
    // Search subscribe pcs.
#if 0
    auto pcc_it = subscribe_pcs_.find(stream_id);
    if (pcc_it != subscribe_pcs_.end()) {
      return pcc_it->second;
    }
#endif
    auto label_it = subscribe_id_label_map_.find(stream_id);
    if (label_it != subscribe_id_label_map_.end()) {
        id = label_it->second;
    }
    else {
        id = stream_id;
    }
    auto pcc_it = subscribe_pcs_.find(id);
    if (pcc_it != subscribe_pcs_.end()) {
        return pcc_it->second;
    }
  }

  {
    std::lock_guard<std::mutex> lock(publish_pcs_mutex_);
    // If stream_id is local stream's ID, find it's label because publish_pcs
    // use
    // label as key.
    auto label_it = publish_id_label_map_.find(stream_id);
    if (label_it != publish_id_label_map_.end()) {
      id = label_it->second;
    } else {
      id = stream_id;
    }
    auto pcc_it = publish_pcs_.find(id);
    if (pcc_it != publish_pcs_.end()) {
      return pcc_it->second;
    }
  }
  LOG(LS_ERROR) << "Cannot find PeerConnectionChannel for specific stream.";
  return nullptr;
}

PeerConnectionChannelConfiguration
ConferenceClient::GetPeerConnectionChannelConfiguration() const {
  PeerConnectionChannelConfiguration config;
  std::vector<webrtc::PeerConnectionInterface::IceServer> ice_servers;
  for(auto it = configuration_.ice_servers.begin(); it!=configuration_.ice_servers.end();++it){
    webrtc::PeerConnectionInterface::IceServer ice_server;
    ice_server.urls=(*it).urls;
    ice_server.username=(*it).username;
    ice_server.password=(*it).password;
    ice_servers.push_back(ice_server);
  }
  config.servers = ice_servers;
  config.media_codec = configuration_.media_codec;
  config.max_audio_bandwidth = configuration_.max_audio_bandwidth;
  config.max_video_bandwidth = configuration_.max_video_bandwidth;
  config.candidate_network_policy =
      (configuration_.candidate_network_policy ==
       ClientConfiguration::CandidateNetworkPolicy::kLowCost)
          ? webrtc::PeerConnectionInterface::CandidateNetworkPolicy::
                kCandidateNetworkPolicyLowCost
          : webrtc::PeerConnectionInterface::CandidateNetworkPolicy::
                kCandidateNetworkPolicyAll;
  return config;
}

void ConferenceClient::OnUserJoined(std::shared_ptr<sio::message> user) {
  TriggerOnUserJoined(user);
}

void ConferenceClient::OnUserLeft(std::shared_ptr<sio::message> user) {
  TriggerOnUserLeft(user);
}

void ConferenceClient::TriggerOnStreamRemoved(sio::message::ptr stream_info) {
  std::string id=stream_info->get_map()["id"]->get_string();
  auto stream_it = added_streams_.find(id);
  auto stream_type = added_stream_type_.find(id);
  if(stream_it==added_streams_.end()||stream_type==added_stream_type_.end()){
    RTC_DCHECK(false);
    LOG(LS_WARNING) << "Invalid stream or type.";
    return;
  }
  auto stream = stream_it->second;
  auto type=stream_type->second;
  {
    const std::lock_guard<std::mutex> lock(observer_mutex_);
    switch (type) {
      case kStreamTypeCamera: {
        std::shared_ptr<RemoteCameraStream> stream_ptr =
            std::static_pointer_cast<RemoteCameraStream>(stream);
        for (auto observers_it = observers_.begin();
             observers_it != observers_.end(); ++observers_it) {
          auto& o = (*observers_it).get();
          event_queue_->PostTask(
              [&o, stream_ptr] { o.OnStreamRemoved(stream_ptr); });
        }
        break;
      }
      case kStreamTypeScreen: {
        std::shared_ptr<RemoteScreenStream> stream_ptr =
            std::static_pointer_cast<RemoteScreenStream>(stream);
        for (auto observers_it = observers_.begin();
             observers_it != observers_.end(); ++observers_it) {
          auto& o = (*observers_it).get();
          event_queue_->PostTask(
              [&o, stream_ptr] { o.OnStreamRemoved(stream_ptr); });
        }
        break;
      }
      case kStreamTypeMix: {
        std::shared_ptr<RemoteMixedStream> stream_ptr =
            std::static_pointer_cast<RemoteMixedStream>(stream);
        for (auto observers_it = observers_.begin();
             observers_it != observers_.end(); ++observers_it) {
          auto& o = (*observers_it).get();
          event_queue_->PostTask(
              [&o, stream_ptr] { o.OnStreamRemoved(stream_ptr); });
        }
        break;
      }
    }
  }
  added_streams_.erase(stream_it);
  added_stream_type_.erase(stream_type);
}

void ConferenceClient::TriggerOnStreamError(
    std::shared_ptr<Stream> stream,
    std::shared_ptr<const ConferenceException> exception) {
  for (auto observers_it = observers_.begin(); observers_it != observers_.end();
       ++observers_it) {
    auto& o = (*observers_it).get();
    auto type = exception->Type();
    auto message = exception->Message();
    event_queue_->PostTask([&o, stream, type, message] {
      std::unique_ptr<ConferenceException> e(
          new ConferenceException(type, message));
      o.OnStreamError(stream, std::move(e));
    });
  }
}

void ConferenceClient::TriggerOnStreamUpdated(sio::message::ptr stream_info) {
  if (!(stream_info && stream_info->get_flag() == sio::message::flag_object &&
        stream_info->get_map()["id"] && stream_info->get_map()["event"] &&
        stream_info->get_map()["id"]->get_flag() == sio::message::flag_string &&
        stream_info->get_map()["event"]->get_flag() ==
            sio::message::flag_object)) {
    RTC_DCHECK(false);
    return;
  }
  std::string id = stream_info->get_map()["id"]->get_string();
  auto event = stream_info->get_map()["event"];
  auto stream_it = added_streams_.find(id);
  auto stream_type = added_stream_type_.find(id);
  if (stream_it == added_streams_.end() ||
      stream_type == added_stream_type_.end()) {
    RTC_DCHECK(false);
    LOG(LS_WARNING) << "Invalid stream or type.";
    return;
  }
  auto stream = stream_it->second;
  auto type = stream_type->second;

  if (event == nullptr || event->get_flag() != sio::message::flag_object
      || event->get_map()["field"] == nullptr
      || event->get_map()["field"]->get_flag() != sio::message::flag_string) {
    LOG(LS_WARNING) << "Invalid stream update event";
    return;
  }
  //TODO(jianlin): Add notification of audio/video active/inactive.
  std::string event_field = event->get_map()["field"]->get_string();
  if (type != kStreamTypeMix || event_field != "video.layout") {
    // TODO(jianjunz): Remove it when this event is supported on other streams.
    LOG(LS_WARNING) << "Stream updated event only supported on mixed stream.";
    return;
  }
  std::shared_ptr<RemoteMixedStream> stream_ptr=std::static_pointer_cast<RemoteMixedStream>(stream);
  stream_ptr->OnVideoLayoutChanged();
}

std::unordered_map<std::string, std::string>
ConferenceClient::AttributesFromStreamInfo(
    std::shared_ptr<sio::message> stream_info) {
  std::unordered_map<std::string, std::string> attributes;
  if (stream_info->get_map().find("attributes") ==
      stream_info->get_map().end()) {
    // TODO: CHECK here. However, to compatible with old version, no CHECK at
    // this time.
    LOG(LS_WARNING) << "Cannot find attributes info.";
    return attributes;
  }
  auto attributes_info = stream_info->get_map()["attributes"];
  if (attributes_info->get_flag() != sio::message::flag_object) {
    // TODO: CHECK here. However, to compatible with old version, no CHECK at
    // this time.
    LOG(LS_WARNING) << "Incorrect attribute format.";
    return attributes;
  }
  auto attribute_map = attributes_info->get_map();
  for (auto const& attribute_pair : attribute_map) {
    if (attribute_pair.second->get_flag() != sio::message::flag_string) {
      RTC_DCHECK(false);
      continue;
    }
    attributes[attribute_pair.first] = attribute_pair.second->get_string();
  }
  return attributes;
}

std::function<void()> ConferenceClient::RunInEventQueue(
    std::function<void()> func) {
  if (func == nullptr)
    return nullptr;;
  std::weak_ptr<ConferenceClient> weak_this = shared_from_this();
  return [func, weak_this] {
    auto that = weak_this.lock();
    if (!that)
      return;
    that->event_queue_->PostTask([func] { func(); });
  };
}

}
}
