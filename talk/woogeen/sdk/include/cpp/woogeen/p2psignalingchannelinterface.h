/*
 * Copyright © 2015 Intel Corporation. All Rights Reserved.
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
#include "woogeen/p2pexception.h"

namespace woogeen {
class P2PSignalingChannelInterfaceObserver {
 public:
  virtual void OnMessage(const std::string& message,
                         const std::string& sender) = 0;
  virtual void OnDisconnected() = 0;
};

class P2PSignalingChannelInterface {
 public:
  virtual void AddObserver(P2PSignalingChannelInterfaceObserver* observer) = 0;
  virtual void Connect(
      const std::string& token,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<P2PException>)> on_failure) = 0;
  virtual void Disconnect(
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<P2PException>)> on_failure) = 0;
  virtual void SendMessage(
      const std::string& message,
      const std::string& target_id,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<P2PException>)> on_failure) = 0;
};
}

#endif  // WOOGEEN_P2P_P2PSIGNALINGCHANNELINTERFACE_H_
