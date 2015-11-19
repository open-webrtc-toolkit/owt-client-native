/*
 * Intel License
 */

#include <thread>
#include "talk/woogeen/sdk/conference/remotemixedstream.h"
#include "talk/woogeen/sdk/conference/conferenceclient.h"
#include "talk/woogeen/sdk/conference/conferenceexception.h"
#include "talk/woogeen/sdk/conference/conferencepeerconnectionchannel.h"

namespace woogeen {

enum ConferenceClient::StreamType : int {
  kStreamTypeCamera = 1,
  kStreamTypeScreen,
  kStreamTypeMix,
};

ConferenceClient::ConferenceClient(
    ConferenceClientConfiguration& configuration,
    std::shared_ptr<ConferenceSocketSignalingChannel> signaling_channel)
    : configuration_(configuration), signaling_channel_(signaling_channel) {
    //optionally enabling HW accleration
#if defined(WEBRTC_WIN)
    if (configuration.hardware_acceleration_ && (configuration.decoder_win_ != nullptr)){
        PeerConnectionDependencyFactory::SetEnableHardwareAcceleration(true, configuration_.decoder_win_);
    }
#endif
}

void ConferenceClient::AddObserver(
    std::shared_ptr<ConferenceClientObserver> observer) {
  observers_.push_back(observer);
}

void ConferenceClient::RemoveObserver(
    std::shared_ptr<ConferenceClientObserver> observer) {
  observers_.erase(std::remove(observers_.begin(), observers_.end(), observer),
                   observers_.end());
}

void ConferenceClient::Join(
    const std::string& token,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  signaling_channel_->AddObserver(*this);
  signaling_channel_->Connect(token, [=](sio::message::ptr info) {
    if (on_success) {
      on_success();
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
    // Trigger OnUserJoin for existed users.
    if (info->get_map()["users"]->get_flag() != sio::message::flag_array) {
      LOG(LS_WARNING) << "Room info doesn't contain valid users.";
    } else {
      auto users = info->get_map()["users"]->get_vector();
      for (auto it = users.begin(); it != users.end(); ++it) {
        auto user = std::make_shared<conference::User>(ParseUser(*it));
        for (auto its = observers_.begin(); its != observers_.end(); ++its) {
          (*its)->OnUserJoined(user);
        }
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
  auto pc_pair = std::make_pair(stream->MediaStream()->label(), pcc);
  publish_pcs_.insert(pc_pair);
  pcc->Publish(stream, on_success, on_failure);
}

void ConferenceClient::Subscribe(
    std::shared_ptr<RemoteStream> stream,
    std::function<void(std::shared_ptr<RemoteStream> stream)> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  LOG(LS_INFO) << "Subscribe";
  if (!CheckNullPointer((uintptr_t)stream.get(), on_failure)) {
    LOG(LS_ERROR) << "Local stream cannot be nullptr.";
    return;
  }
  PeerConnectionChannelConfiguration config =
      GetPeerConnectionChannelConfiguration();
  std::shared_ptr<ConferencePeerConnectionChannel> pcc(
      new ConferencePeerConnectionChannel(config, signaling_channel_));
  auto pc_pair = std::make_pair(stream->Id(), pcc);
  subscribe_pcs_.insert(pc_pair);
  pcc->Subscribe(stream, on_success, on_failure);
}

void ConferenceClient::Unpublish(
    std::shared_ptr<LocalStream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (!CheckNullPointer((uintptr_t)stream.get(), on_failure)) {
    LOG(LS_ERROR) << "Local stream cannot be nullptr.";
    return;
  }
  if (!CheckNullPointer((uintptr_t)stream->MediaStream().get(), on_failure)) {
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
    (*its)->OnMessageReceived(from, message);
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
    (*its)->OnServerDisconnected();
  }
}

void ConferenceClient::OnStreamId(const std::string& id,
                                  const std::string& publish_stream_label) {
  auto id_label = std::make_pair(id, publish_stream_label);
  publish_id_label_map_.insert(id_label);
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
  if(video->get_flag()!=sio::message::flag_object){
    LOG(LS_ERROR) << "Video info for stream " << id << "is invalid, this stream will be ignored.";
    return;
  }
  std::string device = video->get_map()["device"]->get_string();
  if (device == "mcu") {
    auto remote_stream =
        std::make_shared<woogeen::RemoteMixedStream>(id, remote_id);
    for (auto its = observers_.begin(); its != observers_.end(); ++its) {
      (*its)->OnStreamAdded(remote_stream);
    }
    auto stream_pair = std::make_pair(id, remote_stream);
    added_streams_.insert(stream_pair);
    added_stream_type_.insert(std::make_pair(id, StreamType::kStreamTypeMix));
  } else if (device == "screen") {
    auto remote_stream =
        std::make_shared<woogeen::RemoteScreenStream>(id, remote_id);
    for (auto its = observers_.begin(); its != observers_.end(); ++its) {
      (*its)->OnStreamAdded(remote_stream);
    }
    auto stream_pair = std::make_pair(id, remote_stream);
    added_streams_.insert(stream_pair);
    added_stream_type_.insert(std::make_pair(id, StreamType::kStreamTypeScreen));
  } else {
    auto remote_stream =
        std::make_shared<woogeen::RemoteCameraStream>(id, remote_id);
    for (auto its = observers_.begin(); its != observers_.end(); ++its) {
      (*its)->OnStreamAdded(remote_stream);
    }
    auto stream_pair = std::make_pair(id, remote_stream);
    added_streams_.insert(stream_pair);
    added_stream_type_.insert(std::make_pair(id, StreamType::kStreamTypeCamera));
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
  config.servers = configuration_.ice_servers;
  config.media_codec = configuration_.media_codec;
  return config;
}

void ConferenceClient::OnUserJoined(
    std::shared_ptr<const conference::User> user) {
  for (auto its = observers_.begin(); its != observers_.end(); ++its) {
    (*its)->OnUserJoined(user);
  }
}

void ConferenceClient::OnUserLeft(
    std::shared_ptr<const conference::User> user) {
  for (auto its = observers_.begin(); its != observers_.end(); ++its) {
    (*its)->OnUserLeft(user);
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
        (*observers_it)->OnStreamRemoved(stream_ptr);
      }
      break;
    }
    case kStreamTypeScreen:
    {
      std::shared_ptr<RemoteScreenStream> stream_ptr=std::static_pointer_cast<RemoteScreenStream>(stream);
      for(auto observers_it=observers_.begin(); observers_it!=observers_.end();++observers_it){
        (*observers_it)->OnStreamRemoved(stream_ptr);
      }
      break;
    }
    case kStreamTypeMix:
    {
      std::shared_ptr<RemoteMixedStream> stream_ptr=std::static_pointer_cast<RemoteMixedStream>(stream);
      for(auto observers_it=observers_.begin(); observers_it!=observers_.end();++observers_it){
        (*observers_it)->OnStreamRemoved(stream_ptr);
      }
      break;
    }
  }
  added_streams_.erase(stream_it);
  added_stream_type_.erase(stream_type);
}
}
