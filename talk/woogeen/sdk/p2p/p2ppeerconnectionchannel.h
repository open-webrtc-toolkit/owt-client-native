/*
 * Intel License
 */

#ifndef WOOGEEN_P2P_P2PPEERCONNECTIONCHANNEL_H_
#define WOOGEEN_P2P_P2PPEERCONNECTIONCHANNEL_H_

#include "talk/woogeen/sdk/base/peerconnectionhandler.h"
#include "talk/woogeen/sdk/base/signalingsenderinterface.h"
#include "talk/woogeen/sdk/base/signalingreceiverinterface.h"
#include "webrtc/base/json.h"

namespace woogeen {
// An instance of P2PPeerConnectionChannel manages a session for a specified remote client.
class P2PPeerConnectionChannel : public SignalingReceiverInterface{
  public:
    explicit P2PPeerConnectionChannel(const std::string& remote_id, SignalingSenderInterface* sender);
    void Invite(std::function<void()> success, std::function<void(int)> failure);

  protected:
    enum State : int;

  private:
    void OnIncomingMessage(const std::string& message) override;
    void ChangeState(State state);
    void SendSignalingMessage(const Json::Value& data, std::function<void()> success, std::function<void(int)> failure);

    SignalingSenderInterface* signaling_sender_;
    std::string remote_id_;
    State state_;
};

// P2PPeerConnectionChannel callback interface.
// Usually, PeerClient should implement these methods and notify application.
class P2PPeerConnectionChannelObserver {
  public:
    // Triggered when received an invitation.
    virtual void OnInvited() = 0;
    // Triggered when remote user accepted the invitation.
    virtual void OnAccepted() = 0;
    // Triggered when the WebRTC session is ended.
    virtual void OnStopped() = 0;
};
}

#endif // WOOGEEN_P2P_P2PPEERCONNECTIONCHANNEL_H_
