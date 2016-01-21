/*
 * Copyright © 2016 Intel Corporation. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef WOOGEEN_P2P_P2PSIGNALINGCHANNELINTERFACE_H_
#define WOOGEEN_P2P_P2PSIGNALINGCHANNELINTERFACE_H_

#include <functional>
#include <memory>
#include <string>
#include "woogeen/p2p/p2pexception.h"

namespace woogeen {
namespace p2p {
/**
 @brief Signaling channel will notify observer when event triggers.
 */
class P2PSignalingChannelInterfaceObserver {
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
  virtual void OnDisconnected() = 0;
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
  virtual void AddObserver(P2PSignalingChannelInterfaceObserver& observer) = 0;
  /**
   @brief Remove an observer for P2PSignalingChannel
   @param observer An observer instance.
   */
  virtual void RemoveObserver(
      P2PSignalingChannelInterfaceObserver& observer) = 0;
  /**
   @brief Connect to the signaling server
   @param token A token used for connecting signaling server
   */
  virtual void Connect(
      const std::string& token,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<P2PException>)> on_failure) = 0;
  /**
   @brief Disconnect from signaling server.
   */
  virtual void Disconnect(
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<P2PException>)> on_failure) = 0;
  /**
   @brief Send a message to a target client
   @param message Message needs to be send to signaling server
   @param target_id Target user's ID.
   */
  virtual void SendMessage(
      const std::string& message,
      const std::string& target_id,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<P2PException>)> on_failure) = 0;
};
}
}

#endif  // WOOGEEN_P2P_P2PSIGNALINGCHANNELINTERFACE_H_
