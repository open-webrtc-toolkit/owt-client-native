/*
 * Intel License
 */

#include "talk/woogeen/sdk/conference/remotemixedstream.h"
#include "talk/woogeen/sdk/conference/conferenceclient.h"

namespace woogeen {

ConferenceClient::ConferenceClient(std::shared_ptr<ConferenceSignalingChannelInterface> signaling_channel)
    : signaling_channel_(signaling_channel) {
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
      on_success();
    }
    // Trigger OnStreamAdded for existed remote streams.
    std::vector<Json::Value> streams;
    rtc::JsonArrayToValueVector(room_info["streams"], &streams);
    for(auto it=streams.begin();it!=streams.end();++it){  // TODO(jianjun): Only trigger OnStreamAdded for mixed stream for testing.
      LOG(LS_INFO) << "Find streams in the conference.";
      std::string id;
      rtc::GetStringFromJsonObject((*it), "id", &id);
      Json::Value video=(*it)["video"];
      std::string category;
      if(!rtc::GetStringFromJsonObject(video, "category", &category)||category!="mix"){
        auto remote_stream = std::make_shared<woogeen::RemoteCameraStream>(id);
        for (auto its=observers_.begin();its!=observers_.end();++its){
          (*its)->OnStreamAdded(remote_stream);
        }
      }
      else {
        auto remote_stream = std::make_shared<woogeen::RemoteMixedStream>(id);
        for (auto its=observers_.begin();its!=observers_.end();++its){
          (*its)->OnStreamAdded(remote_stream);
        }
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
  std::shared_ptr<ConferencePeerConnectionChannel> pcc(new ConferencePeerConnectionChannel(signaling_channel_));
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
  std::shared_ptr<ConferencePeerConnectionChannel> pcc(new ConferencePeerConnectionChannel(signaling_channel_));
  auto pc_pair = std::make_pair(stream->Id(), pcc);
  publish_pcs_.insert(pc_pair);
  pcc->Subscribe(stream, on_success, on_failure);
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

}
