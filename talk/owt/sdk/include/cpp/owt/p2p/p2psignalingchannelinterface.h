// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_P2P_SIGNALINGCHANNELINTERFACE_H_
#define OWT_P2P_SIGNALINGCHANNELINTERFACE_H_
#include <functional>
#include <memory>
#include <string>
#include "owt/base/exception.h"
namespace owt {
namespace p2p {
using namespace owt::base;
/**
 @brief Signaling channel will notify observer when event triggers.
 */
class P2PSignalingChannelObserver {
 public:
  /**
   @brief This function will be triggered when new message arrives.
   @param message Message received from signaling server.
   @param sender Sender's ID.
   */
  virtual void OnMessage(const std::string& message,
                         const std::string& sender) = 0;
  /**
   @brief This function will be triggered when disconnected from signaling server.
   */
  virtual void OnServerDisconnected() = 0;
};
/**
 @brief Protocol for signaling channel.
 Developers may utilize their own signaling server by implmenting this protocol.
 */
class P2PSignalingChannelInterface {
 public:
  /**
   @brief Add an observer for P2PSignalingChannel
   @param observer An observer instance.
   */
  virtual void AddObserver(P2PSignalingChannelObserver& observer) = 0;
  /**
   @brief Remove an observer for P2PSignalingChannel
   @param observer An observer instance.
   */
  virtual void RemoveObserver(
      P2PSignalingChannelObserver& observer) = 0;
  /**
   @brief Connect to the signaling server
   @param host The URL of signaling server to connect
   @param token A token used for connecting signaling server
   */
  virtual void Connect(
      const std::string& host,
      const std::string& token,
      std::function<void(const std::string&)> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure) = 0;
  /**
   @brief Disconnect from signaling server.
   */
  virtual void Disconnect(
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure) = 0;
  /**
   @brief Send a message to a target client
   @param message Message needs to be send to signaling server
   @param target_id Target user's ID.
   */
  virtual void SendMessage(
      const std::string& message,
      const std::string& target_id,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure) = 0;
};
}
}
#endif  // OWT_P2P_SIGNALINGCHANNELINTERFACE_H_
