/*
 * Intel License
 */

#ifndef WOOGEEN_P2P_P2PPEERCONNECTIONCHANNELOBSERVERCPPIMPL_H_
#define WOOGEEN_P2P_P2PPEERCONNECTIONCHANNELOBSERVERCPPIMPL_H_

#include "talk/woogeen/sdk/p2p/p2ppeerconnectionchannel.h"
#include "talk/woogeen/sdk/include/cpp/woogeen/p2p/peerclient.h"

namespace woogeen {
namespace p2p {

using namespace woogeen::base;

// This class connect a PeerClient and a P2PPeerConnectionChannel, so the
// P2PPeerConnectionChannel can notify PeerClient when event raises.
// Note: an alternative way is make PeerClient derived from
// P2PPeerConnectionChannelObserver, but it will expose
// P2PPeerConnectionChannelObserver's defination to app.
class P2PPeerConnectionChannelObserverCppImpl
    : public P2PPeerConnectionChannelObserver {
 public:
  explicit P2PPeerConnectionChannelObserverCppImpl(PeerClient& peer_client)
      : peer_client_(peer_client) {}

  // Triggered when received an invitation.
  virtual void OnInvited(const std::string& remote_id);
  // Triggered when remote user accepted the invitation.
  virtual void OnAccepted(const std::string& remote_id);
  // Triggered when the WebRTC session is started.
  virtual void OnStarted(const std::string& remote_id);
  // Triggered when the WebRTC session is ended.
  virtual void OnStopped(const std::string& remote_id);
  // Triggered when remote user denied the invitation.
  virtual void OnDenied(const std::string& remote_id);
  // Triggered when remote user send data via data channel.
  // Currently, data is string.
  virtual void OnData(const std::string& remote_id, const std::string& message);
  // Triggered when a new stream is added.
  virtual void OnStreamAdded(
      std::shared_ptr<RemoteCameraStream> stream);
  virtual void OnStreamAdded(
      std::shared_ptr<RemoteScreenStream> stream);
  // Triggered when a remote stream is removed.
  virtual void OnStreamRemoved(
      std::shared_ptr<RemoteCameraStream> stream);
  virtual void OnStreamRemoved(
      std::shared_ptr<RemoteScreenStream> stream);

 private:
  PeerClient& peer_client_;
};
}
}

#endif  // WOOGEEN_P2P_P2PPEERCONNECTIONCHANNELOBSERVERCPPIMPL_H_
