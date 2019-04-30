// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "talk/owt/sdk/p2p/p2ppeerconnectionchannelobservercppimpl.h"
namespace owt {
namespace p2p {
void P2PPeerConnectionChannelObserverCppImpl::OnStarted(
    const std::string& remote_id) {
  peer_client_.OnStarted(remote_id);
}
void P2PPeerConnectionChannelObserverCppImpl::OnStopped(
    const std::string& remote_id) {
  peer_client_.OnStopped(remote_id);
}
void P2PPeerConnectionChannelObserverCppImpl::OnData(
    const std::string& remote_id,
    const std::string& message) {
  peer_client_.OnData(remote_id, message);
}
void P2PPeerConnectionChannelObserverCppImpl::OnStreamAdded(
    std::shared_ptr<RemoteStream> stream) {
  peer_client_.OnStreamAdded(stream);
}
}
}
