/*
 * Intel License
 */

#include <thread>
#include <future>
#include "webrtc/base/base64.h"
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

ConferenceClient::ConferenceClient(const ConferenceClientConfiguration& configuration)
    : configuration_(configuration),
      signaling_channel_(new ConferenceSocketSignalingChannel()) {
}

void ConferenceClient::AddObserver(ConferenceClientObserver& observer) {
  observers_.push_back(observer);
}

void ConferenceClient::RemoveObserver(ConferenceClientObserver& observer) {
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
  std::string token_decoded("");
  if (!rtc::Base64::IsBase64Encoded(token)) {
    LOG(LS_WARNING) << "Passing token with Base64 decoded is deprecated, "
                       "please pass it without modification.";
    token_decoded = token;
  } else {
    if (!rtc::Base64::Decode(token, rtc::Base64::DO_STRICT, &token_decoded,
                             nullptr)) {
      if (on_failure) {
        std::unique_ptr<ConferenceException> e(new ConferenceException(
            ConferenceException::kUnkown, "Invalid token."));
        // TODO: Use async instead.
        on_failure(std::move(e));
      }
      return;
    }
  }
  signaling_channel_->AddObserver(*this);
  signaling_channel_->Connect(token_decoded, [=](sio::message::ptr info) {
    // Get current user's ID.
    std::string user_id;
    if(info->get_map()["clientId"]->get_flag() != sio::message::flag_string) {
      LOG(LS_ERROR) << "Room info doesn't contain client ID.";
      if(on_failure){
        std::unique_ptr<ConferenceException> e(new ConferenceException(
            ConferenceException::kUnkown, "Received unkown message from MCU."));
        // TODO: Use async instead.
        on_failure(std::move(e));
      }
      return;
    } else {
      user_id = info->get_map()["clientId"]->get_string();
    }
    // Trigger OnUserJoin for existed users.
    if (info->get_map()["users"]->get_flag() != sio::message::flag_array) {
      LOG(LS_WARNING) << "Room info doesn't contain valid users.";
    } else {
      auto users = info->get_map()["users"]->get_vector();
      // Get current user's ID and trigger |on_success|. Make sure |on_success|
      // is triggered before any other events because OnUserJoined and
      // OnStreamAdded should be triggered after join a conference.
      if (on_success) {
        auto user_it=users.begin();
        for (auto user_it = users.begin(); user_it != users.end(); user_it++) {
          if ((*user_it)->get_map()["id"]->get_string() == user_id) {
            std::shared_ptr<User> user=std::make_shared<User>(ParseUser(*user_it));
            // TODO: use async instead.
            on_success(std::move(user));
            break;
          }
        }
        if (user_it == users.end()) {
          LOG(LS_ERROR) << "Cannot find current user's info in user list.";
          if (on_failure) {
            std::unique_ptr<ConferenceException> e(
                new ConferenceException(ConferenceException::kUnkown,
                                        "Received unkown message from MCU."));
            // TODO: Use async instead.
            on_failure(std::move(e));
          }
        }
      }
      for (auto it = users.begin(); it != users.end(); ++it) {
        auto user = std::make_shared<conference::User>(ParseUser(*it));
        for (auto its = observers_.begin(); its != observers_.end(); ++its) {
          (*its).get().OnUserJoined(user);
        }
      }
    }
    // Trigger OnStreamAdded for existed remote streams.
    if (info->get_map()["streams"]->get_flag() != sio::message::flag_array) {
      LOG(LS_WARNING) << "Room info doesn't contain valid streams.";
    } else {
      auto streams = info->get_map()["streams"]->get_vector();
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
  if (!CheckNullPointer((uintptr_t)stream.get(), on_failure)) {
    LOG(LS_ERROR) << "Local stream cannot be nullptr.";
    return;
  }
  if (!CheckNullPointer((uintptr_t)stream.get(), on_failure)) {
    LOG(LS_ERROR) << "Cannot publish a local stream without media stream.";
    return;
  }
  PeerConnectionChannelConfiguration config =
      GetPeerConnectionChannelConfiguration();
  std::shared_ptr<ConferencePeerConnectionChannel> pcc(
      new ConferencePeerConnectionChannel(config, signaling_channel_));
  publish_pcs_[stream->MediaStream()->label()] = pcc;
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
  PeerConnectionChannelConfiguration config =
      GetPeerConnectionChannelConfiguration();
  std::shared_ptr<ConferencePeerConnectionChannel> pcc(
      new ConferencePeerConnectionChannel(config, signaling_channel_));
  subscribe_pcs_[stream->Id()] = pcc;
  pcc->Subscribe(stream, options, on_success, on_failure);
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
  auto pcc_it = publish_pcs_.find(stream->MediaStream()->label());
  if (pcc_it == publish_pcs_.end()) {
    LOG(LS_ERROR) << "Cannot find peerconnection channel for stream.";
    if (on_failure != nullptr) {
      std::unique_ptr<ConferenceException> e(new ConferenceException(
          ConferenceException::kUnkown, "Invalid stream."));
      on_failure(std::move(e));
    }
  } else {
    pcc_it->second->Unpublish(stream, [=]() {
      publish_pcs_.erase(pcc_it);
      if (on_success != nullptr)
        on_success();
    }, on_failure);
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
  LOG(LS_INFO) << "About to unsubscribe stream " << stream->Id();
  auto pcc_it = subscribe_pcs_.find(stream->Id());
  if (pcc_it == subscribe_pcs_.end()) {
    LOG(LS_ERROR) << "Cannot find peerconnection channel for stream.";
    if (on_failure != nullptr) {
      std::unique_ptr<ConferenceException> e(new ConferenceException(
          ConferenceException::kUnkown, "Invalid stream."));
      on_failure(std::move(e));
    }
  } else {
    pcc_it->second->Unsubscribe(stream, [=]() {
      subscribe_pcs_.erase(pcc_it);
      if (on_success != nullptr)
        on_success();
    }, on_failure);
  }
}

void ConferenceClient::Send(
    const std::string& message,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  std::string receiver("");
  Send(message, receiver, on_success, on_failure);
}

void ConferenceClient::Send(
    const std::string& message,
    const std::string& receiver,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (message == "") {
    LOG(LS_WARNING) << "Cannot send empty message.";
    if (on_failure != nullptr) {
      std::unique_ptr<ConferenceException> e(new ConferenceException(
          ConferenceException::kUnkown, "Invalid message."));
      on_failure(std::move(e));
    }
    return;
  }
  signaling_channel_->SendCustomMessage(message, receiver, on_success,
                                        on_failure);
}

void ConferenceClient::PlayAudio(
    std::shared_ptr<Stream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  auto pc = GetConferencePeerConnectionChannel(stream);
  if (!CheckNullPointer((uintptr_t)pc.get(), on_failure)) {
    return;
  }
  pc->PlayAudio(stream, on_success, on_failure);
}

void ConferenceClient::PauseAudio(
    std::shared_ptr<Stream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  auto pc = GetConferencePeerConnectionChannel(stream);
  if (!CheckNullPointer((uintptr_t)pc.get(), on_failure)) {
    return;
  }
  pc->PauseAudio(stream, on_success, on_failure);
}

void ConferenceClient::PlayVideo(
    std::shared_ptr<Stream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  auto pc = GetConferencePeerConnectionChannel(stream);
  if (!CheckNullPointer((uintptr_t)pc.get(), on_failure)) {
    return;
  }
  pc->PlayVideo(stream, on_success, on_failure);
}

void ConferenceClient::PauseVideo(
    std::shared_ptr<Stream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  auto pc = GetConferencePeerConnectionChannel(stream);
  if (!CheckNullPointer((uintptr_t)pc.get(), on_failure)) {
    return;
  }
  pc->PauseVideo(stream, on_success, on_failure);
}

void ConferenceClient::Leave(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  publish_id_label_map_.clear();
  publish_pcs_.clear();
  subscribe_pcs_.clear();
  signaling_channel_->Disconnect(on_success, on_failure);
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
                              ? message->get_map()["streamId"]->get_string()
                              : message->get_map()["peerId"]->get_string();
  auto pcc = GetConferencePeerConnectionChannel(stream_id);
  if (pcc == nullptr) {
    LOG(LS_WARNING) << "Received signaling message from unknown sender.";
    return;
  }
  pcc->OnSignalingMessage(message->get_map()["mess"]);
}

void ConferenceClient::OnStreamRemoved(sio::message::ptr stream) {
  TriggerOnStreamRemoved(stream);
}

void ConferenceClient::OnServerDisconnected() {
  for (auto its = observers_.begin(); its != observers_.end(); ++its) {
    (*its).get().OnServerDisconnected();
  }
}

void ConferenceClient::OnStreamId(const std::string& id,
                                  const std::string& publish_stream_label) {
  publish_id_label_map_[id] = publish_stream_label;
  auto pcc = GetConferencePeerConnectionChannel(id);
  RTC_CHECK(pcc != nullptr);
  pcc->SetStreamId(id);
}

bool ConferenceClient::CheckNullPointer(
    uintptr_t pointer,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (pointer)
    return true;
  if (on_failure != nullptr) {
    std::unique_ptr<ConferenceException> e(new ConferenceException(
        ConferenceException::kUnkown, "Nullptr is not allowed."));
    on_failure(std::move(e));
  }
  return false;
}

void ConferenceClient::TriggerOnStreamAdded(sio::message::ptr stream_info) {
  std::string id = stream_info->get_map()["id"]->get_string();
  std::string remote_id = stream_info->get_map()["from"]->get_string();
  auto video = stream_info->get_map()["video"];
  if(video==nullptr||video->get_flag()!=sio::message::flag_object){
    LOG(LS_ERROR) << "Video info for stream " << id << "is invalid, this stream will be ignored.";
    return;
  }
  std::string device;
  if (video->get_map()["device"] != nullptr) {
    device = video->get_map()["device"]->get_string();
  } else {
    device = "camera";
  }
  if (device == "mcu") {
    std::vector<VideoFormat> video_formats;
    // Only mixed streams has multiple resolutions
    if (video->get_map()["resolutions"]) {
      auto resolutions = video->get_map()["resolutions"]->get_vector();
      for (auto it = resolutions.begin(); it != resolutions.end(); ++it) {
        Resolution resolution((*it)->get_map()["width"]->get_int(),
                              (*it)->get_map()["height"]->get_int());
        const VideoFormat video_format(resolution);
        video_formats.push_back(video_format);
      }
    }
    auto remote_stream = std::make_shared<RemoteMixedStream>(
        id, remote_id, video_formats);
    for (auto its = observers_.begin(); its != observers_.end(); ++its) {
      (*its).get().OnStreamAdded(remote_stream);
    }
    added_streams_[id] = remote_stream;
    added_stream_type_[id] = StreamType::kStreamTypeMix;
  } else if (device == "screen") {
    auto remote_stream =
        std::make_shared<RemoteScreenStream>(id, remote_id);
    for (auto its = observers_.begin(); its != observers_.end(); ++its) {
      (*its).get().OnStreamAdded(remote_stream);
    }
    added_streams_[id] = remote_stream;
    added_stream_type_[id] = StreamType::kStreamTypeScreen;
  } else {
    auto remote_stream =
        std::make_shared<RemoteCameraStream>(id, remote_id);
    for (auto its = observers_.begin(); its != observers_.end(); ++its) {
      (*its).get().OnStreamAdded(remote_stream);
    }
    added_streams_[id] = remote_stream;
    added_stream_type_[id] = StreamType::kStreamTypeCamera;
  }
}

conference::User ConferenceClient::ParseUser(
    sio::message::ptr user_message) const {
  std::string id = user_message->get_map()["id"]->get_string();
  std::string user_name = user_message->get_map()["name"]->get_string();
  std::string role = user_message->get_map()["role"]->get_string();
  // TODO(jianjunz): Parse permission info when server side better designed
  // permission structure.
  bool publish = true;
  bool subscribe = true;
  bool record = true;
  conference::Permission permission(publish, subscribe, record);

  conference::User user(id, user_name, role, permission);
  return user;
}

std::shared_ptr<ConferencePeerConnectionChannel>
ConferenceClient::GetConferencePeerConnectionChannel(
    std::shared_ptr<Stream> stream) const {
  if (stream == nullptr) {
    LOG(LS_ERROR) << "Cannot get PeerConnectionChannel for a null stream.";
    return nullptr;
  }
  auto pcc_it = subscribe_pcs_.find(stream->Id());
  if (pcc_it != subscribe_pcs_.end()) {
    return pcc_it->second;
  }
  if (stream->MediaStream() == nullptr) {
    LOG(LS_ERROR) << "Cannot find publish PeerConnectionChannel for a stream "
                     "without media stream.";
    return nullptr;
  }
  pcc_it = publish_pcs_.find(stream->MediaStream()->label());
  if (pcc_it != publish_pcs_.end()) {
    return pcc_it->second;
  }
  LOG(LS_ERROR) << "Cannot find PeerConnectionChannel for specific stream.";
  return nullptr;
}

std::shared_ptr<ConferencePeerConnectionChannel>
ConferenceClient::GetConferencePeerConnectionChannel(
    const std::string& stream_id) const {
  // Search subscribe pcs.
  auto pcc_it = subscribe_pcs_.find(stream_id);
  if (pcc_it != subscribe_pcs_.end()) {
    return pcc_it->second;
  }
  std::string id;
  // If stream_id is local stream's ID, find it's label because publish_pcs use
  // label as key.
  auto label_it = publish_id_label_map_.find(stream_id);
  if (label_it != publish_id_label_map_.end()) {
    id = label_it->second;
  } else {
    id = stream_id;
  }
  pcc_it = publish_pcs_.find(id);
  if (pcc_it != publish_pcs_.end()) {
    return pcc_it->second;
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
  return config;
}

void ConferenceClient::OnUserJoined(
    std::shared_ptr<const conference::User> user) {
  for (auto its = observers_.begin(); its != observers_.end(); ++its) {
    (*its).get().OnUserJoined(user);
  }
}

void ConferenceClient::OnUserLeft(
    std::shared_ptr<const conference::User> user) {
  for (auto its = observers_.begin(); its != observers_.end(); ++its) {
    (*its).get().OnUserLeft(user);
  }
}

void ConferenceClient::TriggerOnStreamRemoved(sio::message::ptr stream_info) {
  std::string id=stream_info->get_map()["id"]->get_string();
  auto stream_it = added_streams_.find(id);
  auto stream_type = added_stream_type_.find(id);
  if(stream_it==added_streams_.end()||stream_type==added_stream_type_.end()){
    ASSERT(false);
    LOG(LS_WARNING) << "Invalid stream or type.";
    return;
  }
  auto stream = stream_it->second;
  auto type=stream_type->second;
  switch(type){
    case kStreamTypeCamera:
    {
      std::shared_ptr<RemoteCameraStream> stream_ptr=std::static_pointer_cast<RemoteCameraStream>(stream);
      for(auto observers_it=observers_.begin(); observers_it!=observers_.end();++observers_it){
        (*observers_it).get().OnStreamRemoved(stream_ptr);
      }
      break;
    }
    case kStreamTypeScreen:
    {
      std::shared_ptr<RemoteScreenStream> stream_ptr=std::static_pointer_cast<RemoteScreenStream>(stream);
      for(auto observers_it=observers_.begin(); observers_it!=observers_.end();++observers_it){
        (*observers_it).get().OnStreamRemoved(stream_ptr);
      }
      break;
    }
    case kStreamTypeMix:
    {
      std::shared_ptr<RemoteMixedStream> stream_ptr=std::static_pointer_cast<RemoteMixedStream>(stream);
      for(auto observers_it=observers_.begin(); observers_it!=observers_.end();++observers_it){
        (*observers_it).get().OnStreamRemoved(stream_ptr);
      }
      break;
    }
  }
  added_streams_.erase(stream_it);
  added_stream_type_.erase(stream_type);
}
}
}
