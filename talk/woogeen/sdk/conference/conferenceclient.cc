/*
 * Intel License
 */

#include "talk/woogeen/sdk/conference/conferenceclient.h"

namespace woogeen {

ConferenceClient::ConferenceClient(std::shared_ptr<ConferenceSignalingChannelInterface> signaling_channel)
    : signaling_channel_(signaling_channel) {
}

void ConferenceClient::Join(const std::string& token, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  signaling_channel_->Connect(token, [=](Json::Value room_info){
    if(on_success){
      on_success();
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
