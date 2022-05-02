// Copyright (C) <2022> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include <string>
#include <unordered_map>
#include "talk/owt/sdk/include/cpp/owt/p2p/p2psignalingchannelinterface.h"
#include "third_party/webrtc/api/task_queue/task_queue_factory.h"
#include "third_party/webrtc/rtc_base/checks.h"

namespace owt {
namespace p2p {
namespace test {

// A simple signaling server for testing. It forwards messages to another client
// connected.
class SimpleSignalingServer {
 public:
  class Visitor {
   public:
    virtual void OnMessage(const std::string& sender,
                           const std::string& message) = 0;
  };

  SimpleSignalingServer();

  void AddClient(const std::string& client_id, Visitor* visitor);
  void SendMessage(const std::string& message,
                   const std::string& sender,
                   const std::string& target);

 private:
  std::unordered_map<std::string, Visitor*> clients_;
  std::unique_ptr<webrtc::TaskQueueFactory> task_queue_factory_;
  // Task queue for forwarding messages.
  std::unique_ptr<webrtc::TaskQueueBase, webrtc::TaskQueueDeleter> task_queue_;
};

// This is a simple signaling channel for testing. Two signaling channels
// connect to a shared signaling server instance for signaling message exchange.
class SimpleSignalingChannel : public owt::p2p::P2PSignalingChannelInterface,
                               SimpleSignalingServer::Visitor {
 public:
  SimpleSignalingChannel(const std::string& client_id,
                         SimpleSignalingServer* server);
  ~SimpleSignalingChannel() = default;

  // Overrides SimpleSignalingObserver::Visitor.
  void OnMessage(const std::string& sender,
                 const std::string& message) override;

  // Overrides owt::p2p::P2PSignalingChannelInterface.
  void AddObserver(P2PSignalingChannelObserver& observer) override;
  void RemoveObserver(P2PSignalingChannelObserver& observer) override;
  void Connect(
      const std::string& host,
      const std::string& token,
      std::function<void(const std::string&)> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure) override;
  void Disconnect(
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure) override;
  void SendMessage(
      const std::string& message,
      const std::string& target_id,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure) override;

 private:
  std::string client_id_;
  SimpleSignalingServer* server_;
  // std::mutex observer_mutex_;
  P2PSignalingChannelObserver* observer_;
};

}  // namespace test
}  // namespace p2p
}  // namespace owt