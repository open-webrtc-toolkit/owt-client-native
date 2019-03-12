// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_P2P_SIGNALINGSENDERINTERFACE_H_
#define OWT_P2P_SIGNALINGSENDERINTERFACE_H_
#include <functional>
#include <memory>
#include <string>
#include "owt/base/exception.h"
namespace owt {
namespace p2p {
/** @cond */
/**
  @brief Interface for signaling sender.
  @details The sender may be a PeerClient/ConferenceClient instance which can
  send out signaling messages created from peerconnection.
*/
class P2PSignalingSenderInterface {
 public:
  virtual ~P2PSignalingSenderInterface() {}
  /// Send a signaling message.
  virtual void SendSignalingMessage(const std::string& message,
                                    const std::string& remote_id,
                                    std::function<void()> success,
                                    std::function<void(std::unique_ptr<owt::base::Exception>)> failure) = 0;
};
}
}
/** @endcond */
#endif  // OWT_P2P_SIGNALINGSENDERINTERFACE_H_
