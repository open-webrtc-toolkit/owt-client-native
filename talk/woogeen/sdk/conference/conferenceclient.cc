/*
 * Intel License
 */

#include <thread>
#include "talk/woogeen/sdk/conference/remotemixedstream.h"
#include "talk/woogeen/sdk/conference/conferenceclient.h"

namespace woogeen {

ConferenceClient::ConferenceClient(ConferenceClientConfiguration& configuration, std::shared_ptr<ConferenceSignalingChannelInterface> signaling_channel)
    : configuration_(configuration),
      signaling_channel_(signaling_channel){
}

void ConferenceClient::AddObserver(std::shared_ptr<ConferenceClientObserver> observer) {
  observers_.push_back(observer);
}

void ConferenceClient::RemoveObserver(std::shared_ptr<ConferenceClientObserver> observer) {
  observers_.erase(std::remove(observers_.begin(), observers_.end(), observer), observers_.end());
}

void ConferenceClient::Join(const std::string& token, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  signaling_channel_->AddObserver(*this);
  signaling_channel_->Connect(token, [=](Json::Value room_info, std::vector<const conference::User> users){
    if(on_success){
      std::thread t(on_success);
      t.detach();
    }
    // Trigger OnStreamAdded for existed remote streams.
    std::vector<Json::Value> streams;
    rtc::JsonArrayToValueVector(room_info["streams"], &streams);
    for(auto it=streams.begin();it!=streams.end();++it){
      LOG(LS_INFO) << "Find streams in the conference.";
      TriggerOnStreamAdded(*it);
    }
    // Trigger OnUserJoin for existed users.
    for(auto it=users.begin();it!=users.end();++it){
      for (auto its=observers_.begin();its!=observers_.end();++its){
        auto user=std::make_shared<const conference::User>(*it);
        (*its)->OnUserJoined(user);
      }
    }
  }, on_failure);
}

void ConferenceClient::Publish(std::shared_ptr<LocalStream> stream, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if(!CheckNullPointer((uintptr_t)stream.get(), on_failure)){
    LOG(LS_ERROR) << "Local stream cannot be nullptr.";
    return;
  }
  if(!CheckNullPointer((uintptr_t)stream.get(), on_failure)){
    LOG(LS_ERROR) << "Cannot publish a local stream without media stream.";
    return;
  }
  PeerConnectionChannelConfiguration config = GetPeerConnectionChannelConfiguration();
  std::shared_ptr<ConferencePeerConnectionChannel> pcc(new ConferencePeerConnectionChannel(config,signaling_channel_));
  auto pc_pair = std::make_pair(stream->MediaStream()->label(), pcc);
  publish_pcs_.insert(pc_pair);
  pcc->Publish(stream, on_success, on_failure);
}

void ConferenceClient::Subscribe(std::shared_ptr<RemoteStream> stream, std::function<void(std::shared_ptr<RemoteStream> stream)> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure){
  LOG(LS_INFO) << "Subscribe";
  if(!CheckNullPointer((uintptr_t)stream.get(), on_failure)){
    LOG(LS_ERROR) << "Local stream cannot be nullptr.";
    return;
  }
  PeerConnectionChannelConfiguration config = GetPeerConnectionChannelConfiguration();
  std::shared_ptr<ConferencePeerConnectionChannel> pcc(new ConferencePeerConnectionChannel(config, signaling_channel_));
  auto pc_pair = std::make_pair(stream->Id(), pcc);
  subscribe_pcs_.insert(pc_pair);
  pcc->Subscribe(stream, on_success, on_failure);
}

void ConferenceClient::Unpublish(std::shared_ptr<LocalStream> stream, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if(!CheckNullPointer((uintptr_t)stream.get(), on_failure)){
    LOG(LS_ERROR) << "Local stream cannot be nullptr.";
    return;
  }
  if(!CheckNullPointer((uintptr_t)stream->MediaStream().get(), on_failure)){
    LOG(LS_ERROR) << "Cannot publish a local stream without media stream.";
    return;
  }
  auto pcc_it=publish_pcs_.find(stream->MediaStream()->label());
  if (pcc_it==publish_pcs_.end()){
    LOG(LS_ERROR) << "Cannot find peerconnection channel for stream.";
    if(on_failure!=nullptr){
      std::unique_ptr<ConferenceException> e(new ConferenceException(ConferenceException::kUnkown, "Invalid stream."));
      on_failure(std::move(e));
    }
  }
  else{
    pcc_it->second->Unpublish(stream, [=](){
      publish_pcs_.erase(pcc_it);
      if(on_success!=nullptr)
        on_success();
    }, on_failure);
  }
}

void ConferenceClient::Unsubscribe(std::shared_ptr<RemoteStream> stream, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure){
  if(!CheckNullPointer((uintptr_t)stream.get(), on_failure)){
    LOG(LS_ERROR) << "Remote stream cannot be nullptr.";
    return;
  }
  LOG(LS_INFO) << "About to unsubscribe stream "<<stream->Id();
  auto pcc_it=subscribe_pcs_.find(stream->Id());
  if(pcc_it==subscribe_pcs_.end()){
    LOG(LS_ERROR) << "Cannot find peerconnection channel for stream.";
    if(on_failure!=nullptr){
      std::unique_ptr<ConferenceException> e(new ConferenceException(ConferenceException::kUnkown, "Invalid stream."));
      on_failure(std::move(e));
    }
  } else {
    pcc_it->second->Unsubscribe(stream, [=](){
      subscribe_pcs_.erase(pcc_it);
      if(on_success!=nullptr)
        on_success();
    }, on_failure);
  }
}

void ConferenceClient::Send(const std::string& message, std::function<void()>on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure){
  std::string receiver("");
  Send(message, receiver, on_success, on_failure);
}

void ConferenceClient::Send(const std::string& message, const std::string& receiver, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure){
  if(message==""){
    LOG(LS_WARNING) << "Cannot send empty message.";
    if(on_failure!=nullptr){
      std::unique_ptr<ConferenceException> e(new ConferenceException(ConferenceException::kUnkown, "Invalid message."));
      on_failure(std::move(e));
    }
    return;
  }
  signaling_channel_->SendCustomMessage(message, receiver, on_success, on_failure);
}

void ConferenceClient::PlayAudio(std::shared_ptr<Stream> stream, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure){
  auto pc=GetConferencePeerConnectionChannel(stream);
  if(!CheckNullPointer((uintptr_t)pc.get(), on_failure)){
    return;
  }
  pc->PlayAudio(stream, on_success, on_failure);
}

void ConferenceClient::PauseAudio(std::shared_ptr<Stream> stream, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure){
  auto pc=GetConferencePeerConnectionChannel(stream);
  if(!CheckNullPointer((uintptr_t)pc.get(), on_failure)){
    return;
  }
  pc->PauseAudio(stream, on_success, on_failure);
}

void ConferenceClient::PlayVideo(std::shared_ptr<Stream> stream, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure){
  auto pc=GetConferencePeerConnectionChannel(stream);
  if(!CheckNullPointer((uintptr_t)pc.get(), on_failure)){
    return;
  }
  pc->PlayVideo(stream, on_success, on_failure);
}

void ConferenceClient::PauseVideo(std::shared_ptr<Stream> stream, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure){
  auto pc=GetConferencePeerConnectionChannel(stream);
  if(!CheckNullPointer((uintptr_t)pc.get(), on_failure)){
    return;
  }
  pc->PauseVideo(stream, on_success, on_failure);
}

void ConferenceClient::Leave(std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure){
  publish_pcs_.clear();
  subscribe_pcs_.clear();
  signaling_channel_->Disconnect(on_success, on_failure);
}

void ConferenceClient::OnStreamAdded(Json::Value stream){
  TriggerOnStreamAdded(stream);
}

void ConferenceClient::OnCustomMessage(std::string& from, std::string& message){
  LOG(LS_INFO) << "ConferenceClient OnCustomMessage";
  for (auto its=observers_.begin();its!=observers_.end();++its){
    (*its)->OnMessageReceived(from, message);
  }
}

void ConferenceClient::OnStreamRemoved(Json::Value stream){
  TriggerOnStreamRemoved(stream);
}

void ConferenceClient::OnServerDisconnected(){
  for (auto its=observers_.begin();its!=observers_.end();++its){
    (*its)->OnServerDisconnected();
  }
}

bool ConferenceClient::CheckNullPointer(uintptr_t pointer, std::function<void(std::unique_ptr<ConferenceException>)>on_failure) {
  if(pointer)
    return true;
  if(on_failure!=nullptr) {
    std::unique_ptr<ConferenceException> e(new ConferenceException(ConferenceException::kUnkown, "Nullptr is not allowed."));
    on_failure(std::move(e));
  }
  return false;
}

void ConferenceClient::TriggerOnStreamAdded(const Json::Value& stream_info){
  std::string id;
  rtc::GetStringFromJsonObject(stream_info, "id", &id);
  Json::Value video=stream_info["video"];
  std::string device;
  std::string remote_id;
  rtc::GetStringFromJsonObject(stream_info, "from", &remote_id);
  if(rtc::GetStringFromJsonObject(video, "device", &device)&&device=="mcu"){
    auto remote_stream = std::make_shared<woogeen::RemoteMixedStream>(id, remote_id);
    for (auto its=observers_.begin();its!=observers_.end();++its){
      (*its)->OnStreamAdded(remote_stream);
    }
    auto stream_pair = std::make_pair(id, remote_stream);
    added_streams_.insert(stream_pair);
  }
  else if (rtc::GetStringFromJsonObject(video, "device", &device)&&device=="screen"){
    auto remote_stream = std::make_shared<woogeen::RemoteScreenStream>(id, remote_id);
    for (auto its=observers_.begin();its!=observers_.end();++its){
      (*its)->OnStreamAdded(remote_stream);
    }
    auto stream_pair = std::make_pair(id, remote_stream);
    added_streams_.insert(stream_pair);
  }
  else {
    auto remote_stream = std::make_shared<woogeen::RemoteCameraStream>(id, remote_id);
    for (auto its=observers_.begin();its!=observers_.end();++its){
      (*its)->OnStreamAdded(remote_stream);
    }
    auto stream_pair = std::make_pair(id, remote_stream);
    added_streams_.insert(stream_pair);
  }
}


std::shared_ptr<ConferencePeerConnectionChannel> ConferenceClient::GetConferencePeerConnectionChannel(std::shared_ptr<Stream> stream) const {
  if(stream==nullptr){
    LOG(LS_ERROR) << "Cannot get PeerConnectionChannel for a null stream.";
    return nullptr;
  }
  auto pcc_it=subscribe_pcs_.find(stream->Id());
  if(pcc_it!=subscribe_pcs_.end()){
    return pcc_it->second;
  }
  if(stream->MediaStream()==nullptr){
    LOG(LS_ERROR) << "Cannot find publish PeerConnectionChannel for a stream without media stream.";
    return nullptr;
  }
  pcc_it=publish_pcs_.find(stream->MediaStream()->label());
  if(pcc_it!=publish_pcs_.end()){
    return pcc_it->second;
  }
  LOG(LS_ERROR) << "Cannot find PeerConnectionChannel for specific stream.";
  return nullptr;
}

PeerConnectionChannelConfiguration ConferenceClient::GetPeerConnectionChannelConfiguration() const {
  PeerConnectionChannelConfiguration config;
  config.servers=configuration_.ice_servers;
  config.media_codec=configuration_.media_codec;
  return config;
}

void ConferenceClient::OnUserJoined(std::shared_ptr<const conference::User> user) {
  for (auto its=observers_.begin();its!=observers_.end();++its){
    (*its)->OnUserJoined(user);
  }
}

void ConferenceClient::OnUserLeft(std::shared_ptr<const conference::User> user) {
  for (auto its=observers_.begin();its!=observers_.end();++its){
    (*its)->OnUserLeft(user);
  }
}

void ConferenceClient::TriggerOnStreamRemoved(const Json::Value& stream_info){
  std::string id;
  rtc::GetStringFromJsonObject(stream_info, "id", &id);
  Json::Value video=stream_info["video"];
  std::string device;
  std::string remote_id;
  rtc::GetStringFromJsonObject(stream_info, "from", &remote_id);
  auto stream_it=added_streams_.find(id);
  if(rtc::GetStringFromJsonObject(video, "device", &device)&&device=="mcu"){
    std::shared_ptr<RemoteMixedStream> stream = std::static_pointer_cast<RemoteMixedStream>(stream_it->second);
    for (auto its=observers_.begin();its!=observers_.end();++its){
      (*its)->OnStreamRemoved(stream);
    }
  }
  else if (rtc::GetStringFromJsonObject(video, "device", &device)&&device=="screen"){
    std::shared_ptr<RemoteScreenStream> stream = std::static_pointer_cast<RemoteScreenStream>(stream_it->second);
    for (auto its=observers_.begin();its!=observers_.end();++its){
      (*its)->OnStreamRemoved(stream);
    }
  }
  else {
    std::shared_ptr<RemoteCameraStream> stream = std::static_pointer_cast<RemoteCameraStream>(stream_it->second);
    for (auto its=observers_.begin();its!=observers_.end();++its){
      (*its)->OnStreamRemoved(stream);
    }
  }
  added_streams_.erase(stream_it);
}

}
