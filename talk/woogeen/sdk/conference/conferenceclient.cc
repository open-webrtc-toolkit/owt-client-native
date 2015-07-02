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
  // TODO(jianjun): implement it.
}

void ConferenceClient::Join(const std::string& token, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  signaling_channel_->Connect(token, [=](Json::Value room_info){
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
  webrtc::PeerConnectionInterface::RTCConfiguration config;
  config.servers=configuration_.ice_servers;
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
  webrtc::PeerConnectionInterface::RTCConfiguration config;
  config.servers=configuration_.ice_servers;
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

void ConferenceClient::Leave(std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure){
  signaling_channel_->Disconnect(on_success, on_failure);
}

void ConferenceClient::OnStreamAdded(Json::Value stream){
  TriggerOnStreamAdded(stream);
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
  std::string category;
  std::string remote_id;
  rtc::GetStringFromJsonObject(stream_info, "from", &remote_id);
  if(!rtc::GetStringFromJsonObject(video, "category", &category)||category!="mix"){
    auto remote_stream = std::make_shared<woogeen::RemoteCameraStream>(id, remote_id);
    for (auto its=observers_.begin();its!=observers_.end();++its){
      (*its)->OnStreamAdded(remote_stream);
    }
  }
  else {
    auto remote_stream = std::make_shared<woogeen::RemoteMixedStream>(id, remote_id);
    for (auto its=observers_.begin();its!=observers_.end();++its){
      (*its)->OnStreamAdded(remote_stream);
    }
  }
}
}
