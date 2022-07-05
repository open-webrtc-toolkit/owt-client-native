// Copyright (C) <2022> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/p2p/test/simple_signaling_channel.h"
#include "third_party/webrtc/api/task_queue/default_task_queue_factory.h"
#include "third_party/webrtc/api/task_queue/task_queue_base.h"
#include "third_party/webrtc/rtc_base/logging.h"
#include "third_party/webrtc/rtc_base/task_utils/to_queued_task.h"

namespace owt {
namespace p2p {
namespace test {

SimpleSignalingServer::SimpleSignalingServer()
    : task_queue_factory_(webrtc::CreateDefaultTaskQueueFactory()),
      task_queue_(task_queue_factory_->CreateTaskQueue(
          "SignalingServer",
          webrtc::TaskQueueFactory::Priority::LOW)) {}

void SimpleSignalingServer::AddClient(const std::string& client_id,
                                      Visitor* visitor) {
  RTC_CHECK(clients_.find(client_id) == clients_.end());
  clients_[client_id] = visitor;
}

void SimpleSignalingServer::SendMessage(const std::string& message,
                                        const std::string& sender,
                                        const std::string& target) {
  task_queue_->PostTask(webrtc::ToQueuedTask([message, sender, target, this] {
    auto client_it = clients_.find(target);
    if (client_it == clients_.end()) {
      return;
    }
    RTC_LOG(LS_VERBOSE) << sender << " -> " << target << ": " << message;
    client_it->second->OnMessage(sender, message);
  }));
}

SimpleSignalingChannel::SimpleSignalingChannel(const std::string& client_id,
                                               SimpleSignalingServer* server)
    : client_id_(client_id), server_(server), observer_(nullptr) {
  RTC_CHECK(server_);
}

void SimpleSignalingChannel::OnMessage(const std::string& sender,
                                       const std::string& message) {
  if (observer_) {
    observer_->OnSignalingMessage(message, sender);
  }
}

void SimpleSignalingChannel::AddObserver(
    P2PSignalingChannelObserver& observer) {
  RTC_CHECK(observer_ == nullptr);
  observer_ = &observer;
}

void SimpleSignalingChannel::RemoveObserver(
    P2PSignalingChannelObserver& observer) {
  observer_ = nullptr;
}

void SimpleSignalingChannel::Connect(
    const std::string& host,
    const std::string& token,
    std::function<void(const std::string&)> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  server_->AddClient(client_id_, this);
  if (on_success) {
    on_success(token);
  }
}

void SimpleSignalingChannel::Disconnect(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {}

void SimpleSignalingChannel::SendMessage(
    const std::string& message,
    const std::string& target_id,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  server_->SendMessage(message, client_id_, target_id);
  if (on_success) {
    on_success();
  }
}

}  // namespace test
}  // namespace p2p
}  // namespace owt