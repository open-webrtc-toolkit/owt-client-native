/*
 * Intel License
 */

#include "talk/woogeen/sdk/conference/conferenceclient.h"

namespace woogeen {

ConferenceClient::ConferenceClient(std::unique_ptr<ConferenceSignalingChannelInterface> signaling_channel) {
  signaling_channel_=std::move(signaling_channel);
}

void ConferenceClient::Join(const std::string& token, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  signaling_channel_->Connect(token, on_success, on_failure);
}

}
