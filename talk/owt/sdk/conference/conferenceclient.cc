// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include <algorithm>
#include <string>
#include "talk/owt/sdk/base/mediautils.h"
#include "talk/owt/sdk/base/stringutils.h"
#include "talk/owt/sdk/conference/conferencepeerconnectionchannel.h"
#include "talk/owt/sdk/include/cpp/owt/base/stream.h"
#include "talk/owt/sdk/include/cpp/owt/conference/conferenceclient.h"
#include "talk/owt/sdk/include/cpp/owt/conference/remotemixedstream.h"
#include "webrtc/api/statstypes.h"
#include "webrtc/rtc_base/third_party/base64/base64.h"
#include "webrtc/rtc_base/criticalsection.h"
#include "webrtc/rtc_base/logging.h"
#include "webrtc/rtc_base/task_queue.h"
using namespace rtc;
namespace owt {
namespace conference {
static const std::unordered_map<std::string, AudioSourceInfo>
    audio_source_names = {{"mic", AudioSourceInfo::kMic},
                          {"screen-cast", AudioSourceInfo::kScreenCast},
                          {"raw-file", AudioSourceInfo::kFile},
                          {"encoded-file", AudioSourceInfo::kFile},
                          {"mcu", AudioSourceInfo::kMixed}};
static const std::unordered_map<std::string, VideoSourceInfo>
    video_source_names = {{"camera", VideoSourceInfo::kCamera},
                          {"screen-cast", VideoSourceInfo::kScreenCast},
                          {"raw-file", VideoSourceInfo::kFile},
                          {"encoded-file", VideoSourceInfo::kFile},
                          {"mcu", VideoSourceInfo::kMixed}};
void Participant::AddObserver(ParticipantObserver& observer) {
  const std::lock_guard<std::mutex> lock(observer_mutex_);
  std::vector<std::reference_wrapper<ParticipantObserver>>::iterator it =
      std::find_if(observers_.begin(), observers_.end(),
                   [&](std::reference_wrapper<ParticipantObserver> o) -> bool {
                     return &observer == &(o.get());
                   });
  if (it != observers_.end()) {
    return;
  }
  observers_.push_back(observer);
}
void Participant::RemoveObserver(ParticipantObserver& observer) {
  const std::lock_guard<std::mutex> lock(observer_mutex_);
  observers_.erase(
      std::find_if(observers_.begin(), observers_.end(),
                   [&](std::reference_wrapper<ParticipantObserver> o) -> bool {
                     return &observer == &(o.get());
                   }));
}
void Participant::TriggerOnParticipantLeft() {
  // Not acquring locks for observer
  for (auto its = observers_.begin(); its != observers_.end(); ++its) {
    (*its).get().OnLeft();
  }
}
void ConferenceInfo::AddParticipant(std::shared_ptr<Participant> participant) {
  if (!ParticipantPresent(participant->Id())) {
    const std::lock_guard<std::mutex> lock(participants_mutex_);
    participants_.push_back(participant);
  }
  return;
}
void ConferenceInfo::AddOrUpdateStream(
    std::shared_ptr<RemoteStream> remote_stream,
    bool& update) {
  update = false;
  std::string stream_id = remote_stream->Id();
  const std::lock_guard<std::mutex> lock(remote_streams_mutex_);
  auto it = std::find_if(remote_streams_.begin(), remote_streams_.end(),
                         [&](std::shared_ptr<RemoteStream> o) -> bool {
                           bool match = (o->Id() == stream_id);
                           if (match)
                             update = true;
                           return match;
                         });
  if (update && it != remote_streams_.end()) {
    (*it)->Capabilities(remote_stream->Capabilities());
    (*it)->Settings(remote_stream->Settings());
    // Attributes is not supported to be updated so we will not update it.
    TriggerOnStreamUpdated(stream_id);
  } else {
    remote_streams_.push_back(remote_stream);
  }
}
void ConferenceInfo::RemoveParticipantById(const std::string& id) {
  const std::lock_guard<std::mutex> lock(participants_mutex_);
  participants_.erase(std::find_if(
      participants_.begin(), participants_.end(),
      [&](std::shared_ptr<Participant> o) -> bool { return o->Id() == id; }));
}
void ConferenceInfo::RemoveStreamById(const std::string& stream_id) {
  const std::lock_guard<std::mutex> lock(remote_streams_mutex_);
  remote_streams_.erase(
      std::find_if(remote_streams_.begin(), remote_streams_.end(),
                   [&](std::shared_ptr<RemoteStream> o) -> bool {
                     return o->Id() == stream_id;
                   }));
}
bool ConferenceInfo::ParticipantPresent(const std::string& participant_id) {
  const std::lock_guard<std::mutex> lock(participants_mutex_);
  for (auto& it : participants_) {
    if (it->Id() == participant_id)
      return true;
  }
  return false;
}
bool ConferenceInfo::RemoteStreamPresent(const std::string& stream_id) {
  const std::lock_guard<std::mutex> lock(remote_streams_mutex_);
  for (auto& it : remote_streams_) {
    if (it->Id() == stream_id)
      return true;
  }
  return false;
}
void ConferenceInfo::TriggerOnParticipantLeft(
    const std::string& participant_id) {
  const std::lock_guard<std::mutex> lock(participants_mutex_);
  for (auto& it : participants_) {
    if (it->Id() == participant_id) {
      it->TriggerOnParticipantLeft();
      break;
    }
  }
}
void ConferenceInfo::TriggerOnStreamEnded(const std::string& stream_id) {
  for (auto& it : remote_streams_) {
    if (it->Id() == stream_id) {
      it->TriggerOnStreamEnded();
      break;
    }
  }
}
void ConferenceInfo::TriggerOnStreamUpdated(const std::string& stream_id) {
  for (auto& it : remote_streams_) {
    if (it->Id() == stream_id) {
      it->TriggerOnStreamUpdated();
      break;
    }
  }
}
void ConferenceInfo::TriggerOnStreamMuteOrUnmute(const std::string& stream_id, 
          owt::base::TrackKind track_kind, bool muted) {
  for (auto& it : remote_streams_) {
    if (it->Id() == stream_id) {
      if (muted) {
        it->TriggerOnStreamMute(track_kind);
      }
      else {
        it->TriggerOnStreamUnmute(track_kind);
      }      
      break;
    }
  }
}
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
    RTC_LOG(LS_INFO) << "Adding duplicate observer.";
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
void ConferenceClient::AddStreamUpdateObserver(
    ConferenceStreamUpdateObserver& observer) {
  const std::lock_guard<std::mutex> lock(stream_update_observer_mutex_);
  std::vector<std::reference_wrapper<ConferenceStreamUpdateObserver>>::iterator
      it = std::find_if(
          stream_update_observers_.begin(), stream_update_observers_.end(),
          [&](std::reference_wrapper<ConferenceStreamUpdateObserver> o)
              -> bool { return &observer == &(o.get()); });
  if (it != stream_update_observers_.end()) {
    RTC_LOG(LS_INFO) << "Adding duplicate observer.";
    return;
  }
  stream_update_observers_.push_back(observer);
}
void ConferenceClient::RemoveStreamUpdateObserver(
    ConferenceStreamUpdateObserver& observer) {
  const std::lock_guard<std::mutex> lock(stream_update_observer_mutex_);
  auto it = std::find_if(
      stream_update_observers_.begin(), stream_update_observers_.end(),
      [&](std::reference_wrapper<ConferenceStreamUpdateObserver> o) -> bool {
        return &observer == &(o.get());
      });
  if (it != stream_update_observers_.end())
    stream_update_observers_.erase(it);
}
void ConferenceClient::Join(
    const std::string& token,
    std::function<void(std::shared_ptr<ConferenceInfo>)> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  if (signaling_channel_connected_) {
    if (on_failure != nullptr) {
      event_queue_->PostTask([on_failure]() {
        std::unique_ptr<Exception> e(
            new Exception(ExceptionType::kConferenceUnknown,
                          "Already connected to conference server."));
        on_failure(std::move(e));
      });
    }
    return;
  }
  std::string token_base64(token);
  if (!StringUtils::IsBase64EncodedString(token)) {
    RTC_LOG(LS_WARNING) << "Passing token with Base64 decoded is deprecated, "
                           "please pass it without modification.";
    token_base64 = rtc::Base64::Encode(token);
  }
  signaling_channel_->AddObserver(*this);
  signaling_channel_->Connect(
      token_base64,
      [=](sio::message::ptr info) {
        signaling_channel_connected_ = true;
        // Get current user's participantId, user ID and role and fill in the
        // ConferenceInfo.
        std::string participant_id, user_id, role;
        if (info->get_map()["id"]->get_flag() != sio::message::flag_string ||
            info->get_map()["user"]->get_flag() != sio::message::flag_string ||
            info->get_map()["role"]->get_flag() != sio::message::flag_string) {
          RTC_LOG(LS_ERROR)
              << "Room info doesn't contain participant's ID/uerID/role.";
          if (on_failure) {
            event_queue_->PostTask([on_failure]() {
              std::unique_ptr<Exception> e(
                  new Exception(ExceptionType::kConferenceUnknown,
                                "Received invalid user info from MCU."));
              on_failure(std::move(e));
            });
          }
          return;
        } else {
          participant_id = info->get_map()["id"]->get_string();
          user_id = info->get_map()["user"]->get_string();
          role = info->get_map()["role"]->get_string();
          const std::lock_guard<std::mutex> lock(conference_info_mutex_);
          if (!current_conference_info_.get()) {
            current_conference_info_.reset(new ConferenceInfo);
            current_conference_info_->self_.reset(
                new Participant(participant_id, role, user_id));
          }
        }
        auto room_info = info->get_map()["room"];
        if (room_info == nullptr ||
            room_info->get_flag() != sio::message::flag_object) {
          RTC_DCHECK(false);
          return;
        }
        if (room_info->get_map()["id"]->get_flag() !=
            sio::message::flag_string) {
          RTC_DCHECK(false);
          return;
        } else {
          current_conference_info_->id_ =
              room_info->get_map()["id"]->get_string();
        }
        // Trigger OnUserJoin for existed users, and also fill in the
        // ConferenceInfo.
        if (room_info->get_map()["participants"]->get_flag() !=
            sio::message::flag_array) {
          RTC_LOG(LS_WARNING) << "Room info doesn't contain valid users.";
        } else {
          auto users = room_info->get_map()["participants"]->get_vector();
          // Make sure |on_success| is triggered before any other events because
          // OnUserJoined and OnStreamAdded should be triggered after join a
          // conference.
          for (auto it = users.begin(); it != users.end(); ++it) {
            TriggerOnUserJoined(*it, true);
          }
        }
        // Trigger OnStreamAdded for existed remote streams, and also fill in
        // the ConferenceInfo.
        if (room_info->get_map()["streams"]->get_flag() !=
            sio::message::flag_array) {
          RTC_LOG(LS_WARNING) << "Room info doesn't contain valid streams.";
        } else {
          auto streams = room_info->get_map()["streams"]->get_vector();
          for (auto it = streams.begin(); it != streams.end(); ++it) {
            RTC_LOG(LS_INFO) << "Find streams in the conference.";
            TriggerOnStreamAdded(*it, true);
          }
        }
        // Invoke the success callback before trigger any participant join or
        // stream added message.
        if (on_success) {
          event_queue_->PostTask(
              [on_success, this]() { on_success(current_conference_info_); });
        }
      },
      on_failure);
}
void ConferenceClient::Publish(
    std::shared_ptr<LocalStream> stream,
    std::function<void(std::shared_ptr<ConferencePublication>)> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  PublishOptions options;
  Publish(stream, options, on_success, on_failure);
}
void ConferenceClient::Publish(
    std::shared_ptr<LocalStream> stream,
    const PublishOptions& options,
    std::function<void(std::shared_ptr<ConferencePublication>)> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  if (!CheckNullPointer((uintptr_t)stream.get(), on_failure)) {
    RTC_LOG(LS_ERROR) << "Local stream cannot be nullptr.";
    return;
  }
  if (!CheckNullPointer((uintptr_t)(stream->MediaStream()), on_failure)) {
    RTC_LOG(LS_ERROR) << "Cannot publish a local stream without media stream.";
    return;
  }
  if (stream->MediaStream()->GetAudioTracks().size() == 0 &&
      stream->MediaStream()->GetVideoTracks().size() == 0) {
    RTC_LOG(LS_ERROR) << "Cannot publish a local stream without audio & video";
    std::string failure_message(
        "Publishing local stream with neither audio nor video.");
    if (on_failure != nullptr) {
      event_queue_->PostTask([on_failure, failure_message]() {
        std::unique_ptr<Exception> e(
            new Exception(ExceptionType::kConferenceUnknown, failure_message));
        on_failure(std::move(e));
      });
    }
    return;
  }
  if (!CheckSignalingChannelOnline(on_failure)) {
    RTC_LOG(LS_ERROR) << "Signaling channel disconnected.";
    return;
  }
  // Reorder SDP according to perference list.
  PeerConnectionChannelConfiguration config =
      GetPeerConnectionChannelConfiguration();
  for (auto codec : options.video) {
    config.video.push_back(VideoEncodingParameters(codec));
  }
  for (auto codec : options.audio) {
    config.audio.push_back(AudioEncodingParameters(codec));
  }
  std::shared_ptr<ConferencePeerConnectionChannel> pcc(
      new ConferencePeerConnectionChannel(config, signaling_channel_,
                                          event_queue_));
  pcc->AddObserver(*this);
  {
    std::lock_guard<std::mutex> lock(publish_pcs_mutex_);
    publish_pcs_.push_back(pcc);
  }
  std::weak_ptr<ConferenceClient> weak_this = shared_from_this();
  std::string stream_id = stream->Id();
  pcc->Publish(stream,
               [on_success, weak_this, stream_id](std::string session_id) {
                 auto that = weak_this.lock();
                 if (!that)
                   return;
                 // map current pcc
                 if (on_success != nullptr) {
                   std::shared_ptr<ConferencePublication> cp(
                       new ConferencePublication(that, session_id, stream_id));
                   on_success(cp);
                 }
               },
               on_failure);
}
void ConferenceClient::Subscribe(
    std::shared_ptr<RemoteStream> stream,
    std::function<void(std::shared_ptr<ConferenceSubscription>)> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  SubscribeOptions options;
  Subscribe(stream, options, on_success, on_failure);
}
void ConferenceClient::Subscribe(
    std::shared_ptr<RemoteStream> stream,
    const SubscribeOptions& options,
    std::function<void(std::shared_ptr<ConferenceSubscription>)> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  if (!CheckNullPointer((uintptr_t)stream.get(), on_failure)) {
    RTC_LOG(LS_ERROR) << "Local stream cannot be nullptr.";
    return;
  }
  if (!CheckSignalingChannelOnline(on_failure)) {
    return;
  }
  RTC_LOG(LS_INFO) << "Stream ID: " << stream->Id();
  if (added_stream_type_.find(stream->Id()) == added_stream_type_.end()) {
    std::string failure_message(
        "Subscribing an invalid stream. Please check whether this stream is "
        "removed.");
    if (on_failure != nullptr) {
      event_queue_->PostTask([on_failure, failure_message]() {
        std::unique_ptr<Exception> e(
            new Exception(ExceptionType::kConferenceUnknown, failure_message));
        on_failure(std::move(e));
      });
    }
    return;
  }
  if (options.video.disabled && options.audio.disabled) {
    std::string failure_message(
        "Subscribing with both audio and video disabled is not allowed.");
    if (on_failure != nullptr) {
      event_queue_->PostTask([on_failure, failure_message]() {
        std::unique_ptr<Exception> e(
            new Exception(ExceptionType::kConferenceUnknown, failure_message));
        on_failure(std::move(e));
      });
    }
    return;
  }
  // Avoid subscribing the same stream twice.
  {
    std::lock_guard<std::mutex> lock(subscribe_pcs_mutex_);
    // Search subscirbe pcs
    auto it = std::find_if(
        subscribe_pcs_.begin(), subscribe_pcs_.end(),
        [&](std::shared_ptr<ConferencePeerConnectionChannel> o) -> bool {
          return o->GetSubStreamId() == stream->Id();
        });
    if (it != subscribe_pcs_.end()) {
      std::string failure_message(
          "The same remote stream has already been subscribed. Subcribe after "
          "it is "
          "unsubscribed");
      if (on_failure != nullptr) {
        event_queue_->PostTask([on_failure, failure_message]() {
          std::unique_ptr<Exception> e(new Exception(
              ExceptionType::kConferenceUnknown, failure_message));
          on_failure(std::move(e));
        });
      }
      return;
    }
  }
  // Reorder SDP according to perference list.
  PeerConnectionChannelConfiguration config =
      GetPeerConnectionChannelConfiguration();
  for (auto codec : options.video.codecs) {
    config.video.push_back(VideoEncodingParameters(codec, 0, false));
  }
  for (auto codec : options.audio.codecs) {
    config.audio.push_back(AudioEncodingParameters(codec, 0));
  }
  std::shared_ptr<ConferencePeerConnectionChannel> pcc(
      new ConferencePeerConnectionChannel(config, signaling_channel_,
                                          event_queue_));
  pcc->AddObserver(*this);
  {
    std::lock_guard<std::mutex> lock(subscribe_pcs_mutex_);
    subscribe_pcs_.push_back(pcc);
  }
  std::weak_ptr<ConferenceClient> weak_this = shared_from_this();
  std::string stream_id = stream->Id();
  pcc->Subscribe(
      stream, options,
      [on_success, weak_this, stream_id](std::string session_id) {
        auto that = weak_this.lock();
        if (!that)
          return;
        // map current pcc
        if (on_success != nullptr) {
          std::shared_ptr<ConferenceSubscription> cp(
              new ConferenceSubscription(that, session_id, stream_id));
          on_success(cp);
        }
      },
      on_failure);
}
void ConferenceClient::UnPublish(
    const std::string& session_id,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  if (!CheckSignalingChannelOnline(on_failure)) {
    return;
  }
  auto pcc = GetConferencePeerConnectionChannel(session_id);
  if (pcc == nullptr) {
    if (on_failure) {
      event_queue_->PostTask([on_failure]() {
        std::unique_ptr<Exception> e(new Exception(
            ExceptionType::kConferenceUnknown, "Invalid publication id."));
        on_failure(std::move(e));
      });
    }
    return;
  }
  pcc->Unpublish(session_id,
                 [=]() {
                   if (on_success != nullptr)
                     event_queue_->PostTask([on_success]() { on_success(); });
                   {
                     std::lock_guard<std::mutex> lock(publish_pcs_mutex_);
                     auto it = publish_pcs_.begin();
                     while (it != publish_pcs_.end()) {
                       if ((*it)->GetSessionId() == session_id) {
                         publish_pcs_.erase(it);
                         break;
                       }
                       ++it;
                     }
                   }
                 },
                 on_failure);
}
void ConferenceClient::UnSubscribe(
    const std::string& session_id,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  if (!CheckSignalingChannelOnline(on_failure)) {
    return;
  }
  auto pcc = GetConferencePeerConnectionChannel(session_id);
  if (pcc == nullptr) {
    if (on_failure) {
      event_queue_->PostTask([on_failure]() {
        std::unique_ptr<Exception> e(new Exception(
            ExceptionType::kConferenceUnknown, "Invalid subsciption id."));
        on_failure(std::move(e));
      });
    }
    return;
  }
  pcc->Unsubscribe(session_id,
                   [=]() {
                     if (on_success != nullptr)
                       event_queue_->PostTask([on_success]() { on_success(); });
                     {
                       std::lock_guard<std::mutex> lock(subscribe_pcs_mutex_);
                       auto it = subscribe_pcs_.begin();
                       while (it != subscribe_pcs_.end()) {
                         if ((*it)->GetSessionId() == session_id) {
                           subscribe_pcs_.erase(it);
                           break;
                         }
                         ++it;
                       }
                       subscribe_id_label_map_.erase(session_id);
                     }
                   },
                   on_failure);
}
void ConferenceClient::Send(
    const std::string& message,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
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
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  if (!CheckSignalingChannelOnline(on_failure)) {
    return;
  }
  signaling_channel_->SendCustomMessage(
      message, receiver, RunInEventQueue(on_success), on_failure);
}
void ConferenceClient::UpdateSubscription(
    const std::string& session_id,
    const std::string& stream_id,
    const SubscriptionUpdateOptions& option,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  if (!CheckSignalingChannelOnline(on_failure)) {
    if (on_failure) {
      event_queue_->PostTask([on_failure]() {
        std::unique_ptr<Exception> e(
            new Exception(ExceptionType::kConferenceUnknown,
                          "Signaling channel stopped. Unable to send "
                          "subscription update to server"));
        on_failure(std::move(e));
      });
    }
    return;
  }
  sio::message::ptr update_message = sio::object_message::create();
  update_message->get_map()["id"] = sio::string_message::create(session_id);
  update_message->get_map()["operation"] =
      sio::string_message::create("update");
  sio::message::ptr update_option = sio::object_message::create();
  sio::message::ptr video_params = sio::object_message::create();
  if (option.video.frameRate != 0) {
    video_params->get_map()["framerate"] =
        sio::int_message::create(option.video.frameRate);
  }
  if (option.video.resolution.width != 0 &&
      option.video.resolution.height != 0) {
    sio::message::ptr resolution_param = sio::object_message::create();
    resolution_param->get_map()["width"] =
        sio::int_message::create(option.video.resolution.width);
    resolution_param->get_map()["height"] =
        sio::int_message::create(option.video.resolution.height);
    video_params->get_map()["resolution"] = resolution_param;
  }
  if (option.video.keyFrameInterval != 0) {
    video_params->get_map()["keyFrameInterval"] =
        sio::int_message::create(option.video.keyFrameInterval);
  }
  if (option.video.bitrateMultiplier != 0) {
    std::string multiplier =
        "x" + std::to_string(option.video.bitrateMultiplier).substr(0, 3);
    video_params->get_map()["bitrate"] =
        sio::string_message::create(multiplier);
  }
  sio::message::ptr video_update = sio::object_message::create();
  video_update->get_map()["parameters"] = video_params;
  video_update->get_map()["from"] = sio::string_message::create(stream_id);
  update_option->get_map()["video"] = video_update;
  update_message->get_map()["data"] = update_option;
  signaling_channel_->SendSubscriptionUpdateMessage(update_message, on_success,
                                                    on_failure);
}
void ConferenceClient::Mute(
    const std::string& session_id,
    TrackKind track_kind,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  if (!CheckSignalingChannelOnline(on_failure)) {
    return;
  }
  auto pcc = GetConferencePeerConnectionChannel(session_id);
  if (pcc == nullptr ||
      (track_kind != TrackKind::kAudio && track_kind != TrackKind::kVideo &&
       track_kind != TrackKind::kAudioAndVideo)) {
    if (on_failure) {
      event_queue_->PostTask([on_failure]() {
        std::unique_ptr<Exception> e(
            new Exception(ExceptionType::kConferenceUnknown,
                          "Invalid session id or track kind."));
        on_failure(std::move(e));
      });
    }
    return;
  }
  switch (track_kind) {
    case TrackKind::kAudio:
      pcc->PauseAudio(on_success, on_failure);
      break;
    case TrackKind::kVideo:
      pcc->PauseVideo(on_success, on_failure);
      break;
    case TrackKind::kAudioAndVideo:
      pcc->PauseAudioVideo(on_success, on_failure);
      break;
    default:
      break;
  }
}
void ConferenceClient::Unmute(
    const std::string& session_id,
    TrackKind track_kind,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  if (!CheckSignalingChannelOnline(on_failure)) {
    return;
  }
  auto pcc = GetConferencePeerConnectionChannel(session_id);
  if (pcc == nullptr ||
      (track_kind != TrackKind::kAudio && track_kind != TrackKind::kVideo &&
       track_kind != TrackKind::kAudioAndVideo)) {
    if (on_failure) {
      event_queue_->PostTask([on_failure]() {
        std::unique_ptr<Exception> e(
            new Exception(ExceptionType::kConferenceUnknown,
                          "Invalid session id or track kind."));
        on_failure(std::move(e));
      });
    }
    return;
  }
  switch (track_kind) {
    case TrackKind::kAudio:
      pcc->PlayAudio(on_success, on_failure);
      break;
    case TrackKind::kVideo:
      pcc->PlayVideo(on_success, on_failure);
      break;
    case TrackKind::kAudioAndVideo:
      pcc->PlayAudioVideo(on_success, on_failure);
      break;
    default:
      break;
  }
}
void ConferenceClient::Leave(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
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
void ConferenceClient::GetConnectionStats(
    const std::string& session_id,
    std::function<void(std::shared_ptr<ConnectionStats>)> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  auto pcc = GetConferencePeerConnectionChannel(session_id);
  if (pcc == nullptr) {
    if (on_failure) {
      event_queue_->PostTask([on_failure]() {
        std::unique_ptr<Exception> e(
            new Exception(ExceptionType::kConferenceUnknown,
                          "Stream is not published or subscribed."));
        on_failure(std::move(e));
      });
    }
    RTC_LOG(LS_WARNING)
        << "Tried to get connection statistowt from unknown stream.";
    return;
  }
  pcc->GetConnectionStats(on_success, on_failure);
}
void ConferenceClient::GetStats(
    const std::string& session_id,
    std::function<void(const std::vector<const webrtc::StatsReport*>& reports)>
        on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  auto pcc = GetConferencePeerConnectionChannel(session_id);
  if (pcc == nullptr) {
    if (on_failure) {
      event_queue_->PostTask([on_failure]() {
        std::unique_ptr<Exception> e(
            new Exception(ExceptionType::kConferenceUnknown,
                          "Stream is not published or subscribed."));
        on_failure(std::move(e));
      });
    }
    RTC_LOG(LS_WARNING)
        << "Tried to get connection statistowt from unknown stream.";
    return;
  }
  pcc->GetStats(on_success, on_failure);
}
void ConferenceClient::OnStreamAdded(sio::message::ptr stream) {
  TriggerOnStreamAdded(stream);
}
void ConferenceClient::OnCustomMessage(std::string& from,
                                       std::string& message,
                                       std::string& to) {
  RTC_LOG(LS_INFO) << "ConferenceClient OnCustomMessage";
  for (auto its = observers_.begin(); its != observers_.end(); ++its) {
    (*its).get().OnMessageReceived(message, from, to);
  }
}
void ConferenceClient::OnSignalingMessage(sio::message::ptr message) {
  // MCU returns inconsistent format for this event. :(
  std::string stream_id = (message->get_map()["peerId"] == nullptr)
                              ? message->get_map()["id"]->get_string()
                              : message->get_map()["peerId"]->get_string();
  // Check the status before delivering to pcc.
  auto soac_status = message->get_map()["status"];
  if (soac_status == nullptr ||
      soac_status->get_flag() != sio::message::flag_string ||
      (soac_status->get_string() != "soac" &&
       soac_status->get_string() != "ready" &&
       soac_status->get_string() != "error")) {
    RTC_NOTREACHED();
    RTC_LOG(LS_WARNING) << "Ignore signaling status except soac/ready/error.";
    return;
  }
  auto pcc = GetConferencePeerConnectionChannel(stream_id);
  if (pcc == nullptr) {
    RTC_LOG(LS_WARNING) << "Received signaling message from unknown sender.";
    return;
  }
  if (soac_status->get_string() == "ready") {
    sio::message::ptr success_msg = sio::string_message::create("success");
    pcc->OnSignalingMessage(success_msg);
    return;
  } else if (soac_status->get_string() == "error") {
    sio::message::ptr failure_msg = sio::string_message::create("failure");
    pcc->OnSignalingMessage(failure_msg);
    return;
  }
  auto soac_data = message->get_map()["data"];
  if (soac_data == nullptr ||
      soac_data->get_flag() != sio::message::flag_object) {
    RTC_NOTREACHED();
    RTC_LOG(LS_WARNING)
        << "Received signaling message without offer, answer or candidate.";
    return;
  }
  pcc->OnSignalingMessage(message->get_map()["data"]);
}
void ConferenceClient::OnStreamRemoved(sio::message::ptr stream) {
  RTC_LOG(LS_INFO) << "Stream removed.";
  TriggerOnStreamRemoved(stream);
}
void ConferenceClient::OnStreamUpdated(sio::message::ptr stream) {
  TriggerOnStreamUpdated(stream);
}
// ConferencePeerConnectionChannel observer implemenation.
void ConferenceClient::OnStreamError(sio::message::ptr stream) {
  if (stream == nullptr || stream->get_flag() != sio::message::flag_object ||
      stream->get_map()["streamId"] == nullptr ||
      stream->get_map()["streamId"]->get_flag() != sio::message::flag_string) {
    RTC_DCHECK(false);
    return;
  }
  const std::string stream_id(stream->get_map()["streamId"]->get_string());
  RTC_LOG(LS_ERROR) << "MCU reports connection failed for stream " << stream_id;
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
    std::shared_ptr<const Exception> exception) {
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
}
void ConferenceClient::OnSubscriptionId(const std::string& subscription_id,
                                        const std::string& stream_id) {
  {
    std::lock_guard<std::mutex> lock(subscribe_pcs_mutex_);
    subscribe_id_label_map_[subscription_id] = stream_id;
  }
  auto pcc = GetConferencePeerConnectionChannel(stream_id);
  RTC_CHECK(pcc != nullptr);
}
bool ConferenceClient::CheckNullPointer(
    uintptr_t pointer,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  std::string failure_message = "Null pointer is not allowed.";
  return CheckNullPointer(pointer, failure_message, on_failure);
}
bool ConferenceClient::CheckNullPointer(
    uintptr_t pointer,
    const std::string& failure_message,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  if (pointer)
    return true;
  if (on_failure != nullptr) {
    event_queue_->PostTask([on_failure, failure_message]() {
      std::unique_ptr<Exception> e(
          new Exception(ExceptionType::kConferenceUnknown, failure_message));
      on_failure(std::move(e));
    });
  }
  return false;
}
bool ConferenceClient::CheckSignalingChannelOnline(
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  if (signaling_channel_connected_)
    return true;
  if (on_failure != nullptr) {
    event_queue_->PostTask([on_failure]() {
      std::unique_ptr<Exception> e(
          new Exception(ExceptionType::kConferenceUnknown,
                        "Conference server is not connected."));
      on_failure(std::move(e));
    });
  }
  return false;
}
void ConferenceClient::ParseStreamInfo(sio::message::ptr stream_info,
                                       bool joining) {
  std::string id = stream_info->get_map()["id"]->get_string();
  std::string view("");
  // owner_id here stands for participantID
  std::string owner_id("");
  std::string video_source("");
  std::string audio_source("");
  bool has_audio = false, has_video = false;
  std::unordered_map<std::string, std::string> attributes;
  auto media_info = stream_info->get_map()["media"];
  if (media_info == nullptr ||
      media_info->get_flag() != sio::message::flag_object) {
    RTC_DCHECK(false);
    RTC_LOG(LS_ERROR) << "Invalid media info from stream " << id
                      << ", this stream will be ignored.";
    return;
  }
  auto type = stream_info->get_map()["type"]->get_string();
  if (type != "mixed" && type != "forward") {
    RTC_LOG(LS_ERROR) << "Invalid stream type.";
    return;
  } else if (type == "mixed") {
    // Get the view info for mixed stream.
    auto view_info_obj = stream_info->get_map()["info"];
    if (view_info_obj != nullptr &&
        view_info_obj->get_flag() == sio::message::flag_object) {
      auto label_obj = view_info_obj->get_map()["label"];
      if (label_obj != nullptr &&
          label_obj->get_flag() == sio::message::flag_string) {
        view = label_obj->get_string();
      }
    }
  } else if (type == "forward") {
    // Get the stream attributes and owner id;
    auto pub_info = stream_info->get_map()["info"];
    if (pub_info == nullptr ||
        pub_info->get_flag() != sio::message::flag_object) {
      RTC_LOG(LS_ERROR) << "Invalid publication info from stream " << id
                        << ", this stream will be ignored";
      return;
    }
    owner_id = pub_info->get_map()["owner"]->get_string();
    attributes = AttributesFromStreamInfo(pub_info);
  }
  SubscriptionCapabilities subscription_capabilities;
  PublicationSettings publication_settings;
  auto audio_info = media_info->get_map()["audio"];
  if (audio_info == nullptr ||
      audio_info->get_flag() != sio::message::flag_object) {
    RTC_LOG(LS_INFO) << "No audio in stream " << id;
  } else {
    // Parse the VideoInfo structure.
    auto audio_source_obj = audio_info->get_map()["source"];
    if (audio_source_obj != nullptr &&
        audio_source_obj->get_flag() == sio::message::flag_string) {
      audio_source = audio_source_obj->get_string();
    }
    auto audio_format_obj = audio_info->get_map()["format"];
    if (audio_format_obj == nullptr ||
        audio_format_obj->get_flag() != sio::message::flag_object) {
      RTC_LOG(LS_ERROR) << "Invalid audio format info in media info";
      return;
    }
    // Main audio capability
    std::string codec;
    unsigned long sample_rate = 0, channel_num = 0;
    auto sample_rate_obj = audio_format_obj->get_map()["sampleRate"];
    auto codec_obj = audio_format_obj->get_map()["codec"];
    auto channel_num_obj = audio_format_obj->get_map()["channelNum"];
    if (codec_obj == nullptr ||
        codec_obj->get_flag() != sio::message::flag_string) {
      RTC_LOG(LS_ERROR) << "codec name in optional audio info invalid.";
      return;
    }
    has_audio = true;
    codec = codec_obj->get_string();
    if (sample_rate_obj != nullptr)
      sample_rate = sample_rate_obj->get_int();
    if (channel_num_obj != nullptr)
      channel_num = audio_format_obj->get_int();
    AudioCodecParameters audio_codec_param(
        MediaUtils::GetAudioCodecFromString(codec), channel_num, sample_rate);
    publication_settings.audio.codec = audio_codec_param;
    subscription_capabilities.audio.codecs.push_back(audio_codec_param);
    // Optional audio capabilities
    auto audio_format_obj_optional = audio_info->get_map()["optional"];
    if (audio_format_obj_optional == nullptr ||
        audio_format_obj_optional->get_flag() != sio::message::flag_object) {
      RTC_LOG(LS_INFO) << "No optional audio info available";
    } else {
      auto audio_format_optional =
          audio_format_obj_optional->get_map()["format"];
      if (audio_format_optional == nullptr ||
          audio_format_optional->get_flag() != sio::message::flag_array) {
        RTC_LOG(LS_INFO) << "Invalid optional audio info";
      } else {
        auto formats = audio_format_optional->get_vector();
        for (auto it = formats.begin(); it != formats.end(); ++it) {
          unsigned long optional_sample_rate = 0, optional_channel_num = 0;
          auto optional_sample_rate_obj = (*it)->get_map()["sampleRate"];
          auto optional_codec_obj = (*it)->get_map()["codec"];
          auto optional_channel_num_obj = (*it)->get_map()["channelNum"];
          if (optional_codec_obj == nullptr ||
              optional_codec_obj->get_flag() != sio::message::flag_string) {
            RTC_LOG(LS_ERROR) << "codec name in optional audio info invalid.";
            return;
          }
          codec = optional_codec_obj->get_string();
          if (codec == "nellymoser") {
            codec = "asao";
          }
          if (optional_sample_rate_obj != nullptr)
            optional_sample_rate = optional_sample_rate_obj->get_int();
          if (optional_channel_num_obj != nullptr)
            optional_channel_num = optional_channel_num_obj->get_int();
          subscription_capabilities.audio.codecs.push_back(
              AudioCodecParameters(MediaUtils::GetAudioCodecFromString(codec),
                                   optional_channel_num, optional_sample_rate));
        }
      }
    }
  }
  auto video_info = media_info->get_map()["video"];
  if (video_info == nullptr ||
      video_info->get_flag() != sio::message::flag_object) {
    RTC_LOG(LS_INFO) << "No audio info in the media info";
  } else {
    // Parse the VideoInfo structure.
    auto video_source_obj = video_info->get_map()["source"];
    if (video_source_obj != nullptr &&
        video_source_obj->get_flag() == sio::message::flag_string) {
      video_source = video_source_obj->get_string();
    }
    auto video_format_obj = video_info->get_map()["format"];
    if (video_format_obj == nullptr ||
        video_format_obj->get_flag() != sio::message::flag_object) {
      RTC_LOG(LS_ERROR) << "Invalid video format info.";
      return;
    } else {
      has_video = true;
      VideoSubscriptionCapabilities video_subscription_capabilities;
      // Parse the video publication settings.
      std::string codec_name =
          video_format_obj->get_map()["codec"]->get_string();
      std::string profile_name("");
      auto profile_name_obj = video_format_obj->get_map()["profile"];
      if (profile_name_obj != nullptr &&
          profile_name_obj->get_flag() == sio::message::flag_string) {
        profile_name = profile_name_obj->get_string();
      }
      VideoPublicationSettings video_publication_settings;
      VideoCodecParameters video_codec_parameters(
          MediaUtils::GetVideoCodecFromString(codec_name), profile_name);
      video_publication_settings.codec = video_codec_parameters;
      video_subscription_capabilities.codecs.push_back(video_codec_parameters);
      auto video_params_obj = video_info->get_map()["parameters"];
      if (video_params_obj != nullptr &&
          video_params_obj->get_flag() == sio::message::flag_object) {
        auto main_resolution = video_params_obj->get_map()["resolution"];
        if (main_resolution != nullptr &&
            main_resolution->get_flag() == sio::message::flag_object) {
          Resolution resolution =
              Resolution(main_resolution->get_map()["width"]->get_int(),
                         main_resolution->get_map()["height"]->get_int());
          video_publication_settings.resolution = resolution;
          video_subscription_capabilities.resolutions.push_back(resolution);
        }
        double frame_rate_num = 0, bitrate_num = 0, keyframe_interval_num = 0;
        auto main_frame_rate = video_params_obj->get_map()["framerate"];
        if (main_frame_rate != nullptr) {
          frame_rate_num = main_frame_rate->get_int();
          video_publication_settings.frame_rate = frame_rate_num;
          video_subscription_capabilities.frame_rates.push_back(frame_rate_num);
        }
        auto main_bitrate = video_params_obj->get_map()["bitrate"];
        if (main_bitrate != nullptr) {
          bitrate_num = main_bitrate->get_int();
          video_publication_settings.bitrate = bitrate_num;
        }
        auto main_keyframe_interval =
            video_params_obj->get_map()["keyFrameInterval"];
        if (main_keyframe_interval != nullptr) {
          keyframe_interval_num = main_keyframe_interval->get_int();
          video_publication_settings.keyframe_interval = keyframe_interval_num;
          video_subscription_capabilities.keyframe_intervals.push_back(
              keyframe_interval_num);
        }
      }
      publication_settings.video = video_publication_settings;
      // Parse the video subscription capabilities.
      auto optional_video_obj = video_info->get_map()["optional"];
      if (optional_video_obj != nullptr &&
          optional_video_obj->get_flag() == sio::message::flag_object) {
        auto optional_video_format_obj =
            optional_video_obj->get_map()["format"];
        if (optional_video_format_obj != nullptr &&
            optional_video_format_obj->get_flag() == sio::message::flag_array) {
          auto formats = optional_video_format_obj->get_vector();
          for (auto it = formats.begin(); it != formats.end(); ++it) {
            std::string optional_codec_name =
                (*it)->get_map()["codec"]->get_string();
            std::string optional_profile_name("");
            auto optional_profile_name_obj = (*it)->get_map()["profile"];
            if (optional_profile_name_obj != nullptr &&
                optional_profile_name_obj->get_flag() ==
                    sio::message::flag_string) {
              optional_profile_name = optional_profile_name_obj->get_string();
            }
            video_subscription_capabilities.codecs.push_back(
                VideoCodecParameters(
                    MediaUtils::GetVideoCodecFromString(optional_codec_name),
                    optional_profile_name));
          }
        }
        auto optional_video_params_obj =
            optional_video_obj->get_map()["parameters"];
        if (optional_video_params_obj != nullptr &&
            optional_video_params_obj->get_flag() ==
                sio::message::flag_object) {
          auto resolution_obj =
              optional_video_params_obj->get_map()["resolution"];
          if (resolution_obj != nullptr &&
              resolution_obj->get_flag() == sio::message::flag_array) {
            auto resolutions = resolution_obj->get_vector();
            for (auto it = resolutions.begin(); it != resolutions.end(); ++it) {
              Resolution resolution =
                  Resolution((*it)->get_map()["width"]->get_int(),
                             (*it)->get_map()["height"]->get_int());
              video_subscription_capabilities.resolutions.push_back(resolution);
            }
          }
          auto framerate_obj =
              optional_video_params_obj->get_map()["framerate"];
          if (framerate_obj != nullptr &&
              framerate_obj->get_flag() == sio::message::flag_array) {
            auto framerates = framerate_obj->get_vector();
            for (auto it = framerates.begin(); it != framerates.end(); ++it) {
              double frame_rate = (*it)->get_int();
              video_subscription_capabilities.frame_rates.push_back(frame_rate);
            }
          }
          auto bitrate_obj = optional_video_params_obj->get_map()["bitrate"];
          if (bitrate_obj != nullptr &&
              bitrate_obj->get_flag() == sio::message::flag_array) {
            auto bitrates = bitrate_obj->get_vector();
            for (auto it = bitrates.begin(); it != bitrates.end(); ++it) {
              std::string bitrate_mul = (*it)->get_string();
              // The bitrate multiplier is in the form of "x1.0" and we need to
              // strip the "x" here.
              video_subscription_capabilities.bitrate_multipliers.push_back(
                  std::stod(bitrate_mul.substr(1)));
            }
          }
          auto keyframe_interval_obj =
              optional_video_params_obj->get_map()["keyFrameInterval"];
          if (keyframe_interval_obj != nullptr &&
              keyframe_interval_obj->get_flag() == sio::message::flag_array) {
            auto keyframe_intervals = keyframe_interval_obj->get_vector();
            for (auto it = keyframe_intervals.begin();
                 it != keyframe_intervals.end(); ++it) {
              double keyframe_interval = (*it)->get_int();
              video_subscription_capabilities.keyframe_intervals.push_back(
                  keyframe_interval);
            }
          }
        }
        subscription_capabilities.video = video_subscription_capabilities;
      }
    }
  }
  // Now that all information needed for PublicationSettings and
  // SubscriptionCapabilities have been gathered, we construct remote streams.
  bool updated = false;
  if (type == "forward") {
    AudioSourceInfo audio_source_info(AudioSourceInfo::kUnknown);
    VideoSourceInfo video_source_info(VideoSourceInfo::kUnknown);
    auto audio_source_it = audio_source_names.find(audio_source);
    if (audio_source_it != audio_source_names.end()) {
      audio_source_info = audio_source_it->second;
    }
    auto video_source_it = video_source_names.find(video_source);
    if (video_source_it != video_source_names.end()) {
      video_source_info = video_source_it->second;
    }
    if (video_source != "screen-cast") {
      auto remote_stream = std::make_shared<RemoteStream>(
          id, owner_id, subscription_capabilities, publication_settings);
      remote_stream->has_audio_ = has_audio;
      remote_stream->has_video_ = has_video;
      remote_stream->Attributes(attributes);
      remote_stream->source_.audio = audio_source_info;
      remote_stream->source_.video = video_source_info;
      added_streams_[id] = remote_stream;
      added_stream_type_[id] = StreamType::kStreamTypeCamera;
      {
        const std::lock_guard<std::mutex> lock(stream_update_observer_mutex_);
        current_conference_info_->AddOrUpdateStream(remote_stream, updated);
        if (!joining && !updated) {
          for (auto its = observers_.begin(); its != observers_.end(); ++its) {
            auto& o = (*its).get();
            event_queue_->PostTask(
                [&o, remote_stream] { o.OnStreamAdded(remote_stream); });
          }
        }
      }
    } else {
      auto remote_stream = std::make_shared<RemoteStream>(
          id, owner_id, subscription_capabilities, publication_settings);
      RTC_LOG(LS_INFO) << "OnStreamAdded: screen stream.";
      remote_stream->has_audio_ = has_audio;
      remote_stream->has_video_ = true;
      remote_stream->Attributes(attributes);
      remote_stream->source_.audio = audio_source_info;
      remote_stream->source_.video = video_source_info;
      added_streams_[id] = remote_stream;
      added_stream_type_[id] = StreamType::kStreamTypeScreen;
      {
        const std::lock_guard<std::mutex> lock(stream_update_observer_mutex_);
        current_conference_info_->AddOrUpdateStream(remote_stream, updated);
        if (!joining && !updated) {
          for (auto its = observers_.begin(); its != observers_.end(); ++its) {
            auto& o = (*its).get();
            event_queue_->PostTask(
                [&o, remote_stream] { o.OnStreamAdded(remote_stream); });
          }
        }
      }
    }
  } else if (type == "mixed") {
    std::string remote_id("mcu");  // Not used.
    owner_id = "mcu";
    auto remote_stream = std::make_shared<RemoteMixedStream>(
        id, owner_id, view, subscription_capabilities, publication_settings);
    RTC_LOG(LS_INFO) << "OnStreamAdded: mixed stream.";
    remote_stream->has_audio_ = has_audio;
    remote_stream->has_video_ = has_video;
    remote_stream->source_.audio = AudioSourceInfo::kMixed;
    remote_stream->source_.video = VideoSourceInfo::kMixed;
    added_streams_[id] = remote_stream;
    added_stream_type_[id] = StreamType::kStreamTypeMix;
    {
      const std::lock_guard<std::mutex> lock(stream_update_observer_mutex_);
      current_conference_info_->AddOrUpdateStream(remote_stream, updated);
      if (!joining) {
        for (auto its = observers_.begin(); its != observers_.end(); ++its) {
          auto& o = (*its).get();
          event_queue_->PostTask(
              [&o, remote_stream] { o.OnStreamAdded(remote_stream); });
        }
      }
    }
  }
}
void ConferenceClient::TriggerOnStreamAdded(sio::message::ptr stream_info,
                                            bool joining) {
  ParseStreamInfo(stream_info, joining);
}
void ConferenceClient::TriggerOnUserJoined(sio::message::ptr user_info,
                                           bool joining) {
  Participant* user_raw;
  if (ParseUser(user_info, &user_raw)) {
    std::shared_ptr<Participant> user(user_raw);
    current_conference_info_->AddParticipant(user);
    if (!joining) {
      const std::lock_guard<std::mutex> lock(observer_mutex_);
      for (auto its = observers_.begin(); its != observers_.end(); ++its) {
        auto& o = (*its).get();
        event_queue_->PostTask([&o, user] { o.OnParticipantJoined(user); });
      }
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
  current_conference_info_->TriggerOnParticipantLeft(user_id);
  current_conference_info_->RemoveParticipantById(user_id);
}
bool ConferenceClient::ParseUser(sio::message::ptr user_message,
                                 Participant** participant) const {
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
  *participant = new Participant(id, role, user_name);
  return true;
}
std::shared_ptr<ConferencePeerConnectionChannel>
ConferenceClient::GetConferencePeerConnectionChannel(
    const std::string& session_id) const {
  {
    std::lock_guard<std::mutex> lock(subscribe_pcs_mutex_);
    // Search subscribe pcs.
    auto it = std::find_if(
        subscribe_pcs_.begin(), subscribe_pcs_.end(),
        [&](std::shared_ptr<ConferencePeerConnectionChannel> o) -> bool {
          return o->GetSessionId() == session_id;
        });
    if (it != subscribe_pcs_.end()) {
      return *it;
    }
  }
  {
    std::lock_guard<std::mutex> lock(publish_pcs_mutex_);
    // Search publish pcs
    auto it = std::find_if(
        publish_pcs_.begin(), publish_pcs_.end(),
        [&](std::shared_ptr<ConferencePeerConnectionChannel> o) -> bool {
          return o->GetSessionId() == session_id;
        });
    if (it != publish_pcs_.end()) {
      return *it;
    }
  }
  RTC_LOG(LS_ERROR) << "Cannot find PeerConnectionChannel for specific session";
  return nullptr;
}
PeerConnectionChannelConfiguration
ConferenceClient::GetPeerConnectionChannelConfiguration() const {
  PeerConnectionChannelConfiguration config;
  std::vector<webrtc::PeerConnectionInterface::IceServer> ice_servers;
  for (auto it = configuration_.ice_servers.begin();
       it != configuration_.ice_servers.end(); ++it) {
    webrtc::PeerConnectionInterface::IceServer ice_server;
    ice_server.urls = (*it).urls;
    ice_server.username = (*it).username;
    ice_server.password = (*it).password;
    ice_servers.push_back(ice_server);
  }
  config.servers = ice_servers;
  config.candidate_network_policy =
      (configuration_.candidate_network_policy ==
       ClientConfiguration::CandidateNetworkPolicy::kLowCost)
          ? webrtc::PeerConnectionInterface::CandidateNetworkPolicy::
                kCandidateNetworkPolicyLowCost
          : webrtc::PeerConnectionInterface::CandidateNetworkPolicy::
                kCandidateNetworkPolicyAll;
  config.continual_gathering_policy =
      PeerConnectionInterface::ContinualGatheringPolicy::GATHER_CONTINUALLY;
  return config;
}
void ConferenceClient::OnUserJoined(std::shared_ptr<sio::message> user) {
  TriggerOnUserJoined(user);
}
void ConferenceClient::OnUserLeft(std::shared_ptr<sio::message> user) {
  TriggerOnUserLeft(user);
}
void ConferenceClient::TriggerOnStreamRemoved(sio::message::ptr stream_info) {
  std::string id = stream_info->get_map()["id"]->get_string();
  auto stream_it = added_streams_.find(id);
  auto stream_type = added_stream_type_.find(id);
  if (stream_it == added_streams_.end() ||
      stream_type == added_stream_type_.end()) {
    RTC_LOG(LS_WARNING) << "Invalid stream or type.";
    return;
  }
  added_streams_.erase(stream_it);
  added_stream_type_.erase(stream_type);
  current_conference_info_->TriggerOnStreamEnded(id);
  current_conference_info_->RemoveStreamById(id);
  const std::lock_guard<std::mutex> lock(stream_update_observer_mutex_);
  for (auto its = stream_update_observers_.begin();
       its != stream_update_observers_.end(); ++its) {
    (*its).get().OnStreamRemoved(id);
  }
}
void ConferenceClient::TriggerOnStreamError(
    std::shared_ptr<Stream> stream,
    std::shared_ptr<const Exception> exception) {
  const std::lock_guard<std::mutex> lock(stream_update_observer_mutex_);
  for (auto its = stream_update_observers_.begin();
       its != stream_update_observers_.end(); ++its) {
    (*its).get().OnStreamError(exception->Message());
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
    RTC_LOG(LS_WARNING) << "Invalid stream or type.";
    return;
  }
  auto stream = stream_it->second;
  auto type = stream_type->second;
  if (event == nullptr || event->get_flag() != sio::message::flag_object ||
      event->get_map()["field"] == nullptr ||
      event->get_map()["field"]->get_flag() != sio::message::flag_string) {
    RTC_LOG(LS_WARNING) << "Invalid stream update event";
    return;
  }
  // TODO(jianlin): Add notification of audio/video active/inactive.
  std::string event_field = event->get_map()["field"]->get_string();
  if (type == kStreamTypeMix && event_field == "video.layout") {
    std::shared_ptr<RemoteMixedStream> stream_ptr =
        std::static_pointer_cast<RemoteMixedStream>(stream);
    stream_ptr->OnVideoLayoutChanged();
    return;
  } else if (type == kStreamTypeMix && event_field == "activeInput") {
    auto value = event->get_map()["value"];
    std::string activeAudioInputStreamId = value->get_string();
    std::shared_ptr<RemoteMixedStream> stream_ptr = std::static_pointer_cast<RemoteMixedStream>(stream);
    stream_ptr->OnActiveInputChanged(activeAudioInputStreamId);
    return;
  } else if (event_field == "audio.status" || event_field == "video.status") {
    auto value = event->get_map()["value"];
    if (value == nullptr || value->get_flag() != sio::message::flag_string) {
      RTC_LOG(LS_WARNING) << "Invalid stream update data value";
      return;
    }
    std::string status_value = value->get_string();
    if (status_value != "active" && status_value != "inactive") {
      RTC_LOG(LS_WARNING) << "Invalid stream update status";
      return;
    }
    TrackKind track_kind =
        (event_field == "audio.status") ? TrackKind::kAudio : TrackKind::kVideo;
    bool muted = (status_value == "inactive") ? true : false;
    for (auto its = stream_update_observers_.begin();
         its != stream_update_observers_.end(); ++its) {
      (*its).get().OnStreamMuteOrUnmute(id, track_kind, muted);
    }
    current_conference_info_->TriggerOnStreamMuteOrUnmute(id, track_kind, muted);
  } else if (event_field == ".") {
    // The value field contains an update to stream info
    auto value = event->get_map()["value"];
    if (value == nullptr || value->get_flag() != sio::message::flag_object) {
      RTC_LOG(LS_WARNING) << "Invalid VideoInfo update value";
      return;
    } else {
      ParseStreamInfo(value);
    }
  }
}
std::unordered_map<std::string, std::string>
ConferenceClient::AttributesFromStreamInfo(
    std::shared_ptr<sio::message> stream_info) {
  std::unordered_map<std::string, std::string> attributes;
  if (stream_info->get_map().find("attributes") ==
      stream_info->get_map().end()) {
    // TODO: CHECK here. However, to compatible with old version, no CHECK at
    // this time.
    RTC_LOG(LS_WARNING) << "Cannot find attributes info.";
    return attributes;
  }
  auto attributes_info = stream_info->get_map()["attributes"];
  if (attributes_info->get_flag() != sio::message::flag_object) {
    // TODO: CHECK here. However, to compatible with old version, no CHECK at
    // this time.
    RTC_LOG(LS_WARNING) << "Incorrect attribute format.";
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
    return nullptr;
  ;
  std::weak_ptr<ConferenceClient> weak_this = shared_from_this();
  return [func, weak_this] {
    auto that = weak_this.lock();
    if (!that)
      return;
    that->event_queue_->PostTask([func] { func(); });
  };
}
}  // namespace conference
}  // namespace owt
