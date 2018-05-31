/*
 * Intel License
 */

#include "webrtc/rtc_base/checks.h"
#include "talk/ics/sdk/p2p/p2psignalingsenderimpl.h"

namespace ics {
namespace p2p {

P2PSignalingSenderImpl::P2PSignalingSenderImpl(
    P2PSignalingSenderInterface* sender) {
  sender_ = sender;
}

P2PSignalingSenderImpl::~P2PSignalingSenderImpl() {}

void P2PSignalingSenderImpl::SendSignalingMessage(
    const std::string& message,
    const std::string& remote_id,
    std::function<void()> success,
    std::function<void(int)> failure) {
  RTC_CHECK(sender_);
  sender_->SendSignalingMessage(message, remote_id, success, failure);
}

}
}
