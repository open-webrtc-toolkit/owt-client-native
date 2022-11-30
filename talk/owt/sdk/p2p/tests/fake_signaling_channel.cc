// Copyright (C) <2022> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/p2p/tests/fake_signaling_channel.h"
#include "third_party/webrtc/rtc_base/checks.h"
#include "third_party/webrtc/rtc_base/logging.h"

namespace owt {
namespace p2p {
namespace test {
void FakeSignalingChannel::AddObserver(
    owt::p2p::P2PSignalingChannelObserver& observer) {
  RTC_CHECK(!client2_);
  if (!client1_) {
    client1_ = &observer;
  } else {
    client2_ = &observer;
  }
}

void FakeSignalingChannel::Connect(
    const std::string& host,
    const std::string& token,
    std::function<void(const std::string&)> on_success,
    std::function<void(std::unique_ptr<owt::base::Exception>)> on_failure) {
  if (token != "client1" && token != "client2") {
    on_failure(std::make_unique<owt::base::Exception>(
        owt::base::ExceptionType::kP2PConnectionAuthFailed,
        "Fake signaling channel only supports "
        "clients with name client1 or client2."));
    return;
  }
  // It doesn't actually verify if it's client1 or client2.
  on_success(token);
}

void FakeSignalingChannel::Disconnect(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<owt::base::Exception>)> on_failure) {
  on_success();
}

void FakeSignalingChannel::SendMessage(
    const std::string& message,
    const std::string& target_id,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<owt::base::Exception>)> on_failure) {
  task_queue_->PostTask([this, message, target_id] {
    if (target_id == "client1") {
      client1_->OnSignalingMessage(message, "client2");
    } else {
      client2_->OnSignalingMessage(message, "client1");
    }
  });
  RTC_LOG(LS_INFO) << "->" << target_id << ": " << message;
}

}  // namespace test
}  // namespace p2p
}  // namespace owt