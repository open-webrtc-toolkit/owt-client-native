// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OMS_P2P_SIGNALINGRECEIVERINTERFACE_H_
#define OMS_P2P_SIGNALINGRECEIVERINTERFACE_H_
namespace oms {
namespace p2p {
/** @cond */
/**
  @brief Interface for signaling receiver.
  @details The receiver may be a peerconnection instance which can deal with the
  message received.
*/
class P2PSignalingReceiverInterface {
 public:
  /// Received signaling message.
  virtual void OnIncomingSignalingMessage(const std::string& message) = 0;
};
}
}
/** @endcond */
#endif  // OMS_P2P_SIGNALINGRECEIVERINTERFACE_H_
