// Copyright (C) <2022> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_P2P_TESTS_FAKE_SIGNALING_CHANNEL_H_
#define OWT_P2P_TESTS_FAKE_SIGNALING_CHANNEL_H_

#include "owt/base/exception.h"
#include "owt/p2p/p2pclient.h"
#include "owt/p2p/p2psignalingchannelinterface.h"
#include "third_party/webrtc/api/task_queue/task_queue_base.h"

namespace owt {
namespace p2p {
namespace test {
// FakeSignalingChannel implements P2PSignalingChannelInterface for transmitting
// signaling messages between two P2P clients. Two P2P clients shares the same
// signaling channel. The first one is client1, and the second one is client2.
class FakeSignalingChannel : public owt::p2p::P2PSignalingChannelInterface {
 public:
  FakeSignalingChannel(std::unique_ptr<webrtc::TaskQueueBase,
                                       webrtc::TaskQueueDeleter> task_queue)
      : task_queue_(std::move(task_queue)),
        client1_(nullptr),
        client2_(nullptr) {}
  ~FakeSignalingChannel() override = default;

  void AddObserver(owt::p2p::P2PSignalingChannelObserver& observer) override;
  void Connect(
      const std::string& host,
      const std::string& token,
      std::function<void(const std::string&)> on_success,
      std::function<void(std::unique_ptr<owt::base::Exception>)> on_failure);
  void Disconnect(std::function<void()> on_success,
                  std::function<void(std::unique_ptr<owt::base::Exception>)>
                      on_failure) override;
  void SendMessage(const std::string& message,
                   const std::string& target_id,
                   std::function<void()> on_success,
                   std::function<void(std::unique_ptr<owt::base::Exception>)>
                       on_failure) override;

 private:
  void RemoveObserver(
      owt::p2p::P2PSignalingChannelObserver& observer) override {}

  std::unique_ptr<webrtc::TaskQueueBase, webrtc::TaskQueueDeleter> task_queue_;
  owt::p2p::P2PSignalingChannelObserver* client1_;
  owt::p2p::P2PSignalingChannelObserver* client2_;
};
}  // namespace test
}  // namespace p2p
}  // namespace owt

#endif  // OWT_P2P_TESTS_FAKE_SIGNALING_CHANNEL_H_