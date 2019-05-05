// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef WOOGEEN_P2P_P2PPEERCONNECTIONCHANNELOBSERVERCPPIMPL_H_
#define WOOGEEN_P2P_P2PPEERCONNECTIONCHANNELOBSERVERCPPIMPL_H_
#include "talk/owt/sdk/p2p/p2ppeerconnectionchannel.h"
#include "talk/owt/sdk/include/cpp/owt/p2p/p2pclient.h"
namespace owt {
namespace p2p {
using namespace owt::base;
// This class connects a P2PClient and a P2PPeerConnectionChannel, so the
// P2PPeerConnectionChannel can notify P2PClient when event raises.
// Note: an alternative way is make P2PClient derived from
// P2PPeerConnectionChannelObserver, but it will expose
// P2PPeerConnectionChannelObserver's defination to app.
class P2PPeerConnectionChannelObserverCppImpl
    : public P2PPeerConnectionChannelObserver {
 public:
  explicit P2PPeerConnectionChannelObserverCppImpl(P2PClient& peer_client)
      : peer_client_(peer_client) {}
  // Triggered when remote user send data via data channel.
  // Currently, data is string type.
  virtual void OnMessageReceived(const std::string& remote_id, const std::string& message);
  // Triggered when a new stream is added.
  virtual void OnStreamAdded(std::shared_ptr<RemoteStream> stream);
 private:
  P2PClient& peer_client_;
};
}
}
#endif  // WOOGEEN_P2P_P2PPEERCONNECTIONCHANNELOBSERVERCPPIMPL_H_
