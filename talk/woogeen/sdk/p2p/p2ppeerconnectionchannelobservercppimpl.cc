/*
 * Intel License
 */

#include "talk/woogeen/sdk/p2p/p2ppeerconnectionchannelobservercppimpl.h"

namespace woogeen {
namespace p2p {

void P2PPeerConnectionChannelObserverCppImpl::OnInvited(
    const std::string& remote_id) {
  peer_client_.OnInvited(remote_id);
}

void P2PPeerConnectionChannelObserverCppImpl::OnAccepted(
    const std::string& remote_id) {
  peer_client_.OnAccepted(remote_id);
}

void P2PPeerConnectionChannelObserverCppImpl::OnStarted(
    const std::string& remote_id) {
  peer_client_.OnStarted(remote_id);
}

void P2PPeerConnectionChannelObserverCppImpl::OnStopped(
    const std::string& remote_id) {
  peer_client_.OnStopped(remote_id);
}

void P2PPeerConnectionChannelObserverCppImpl::OnDenied(
    const std::string& remote_id) {
  peer_client_.OnDenied(remote_id);
}

void P2PPeerConnectionChannelObserverCppImpl::OnData(
    const std::string& remote_id,
    const std::string& message) {
  peer_client_.OnData(remote_id, message);
}

void P2PPeerConnectionChannelObserverCppImpl::OnStreamAdded(
    std::shared_ptr<RemoteCameraStream> stream) {
  peer_client_.OnStreamAdded(stream);
}
void P2PPeerConnectionChannelObserverCppImpl::OnStreamAdded(
    std::shared_ptr<RemoteScreenStream> stream) {
  peer_client_.OnStreamAdded(stream);
}

void P2PPeerConnectionChannelObserverCppImpl::OnStreamRemoved(
    std::shared_ptr<RemoteCameraStream> stream) {
  peer_client_.OnStreamRemoved(stream);
}
void P2PPeerConnectionChannelObserverCppImpl::OnStreamRemoved(
    std::shared_ptr<RemoteScreenStream> stream) {
  peer_client_.OnStreamRemoved(stream);
}
}
}
