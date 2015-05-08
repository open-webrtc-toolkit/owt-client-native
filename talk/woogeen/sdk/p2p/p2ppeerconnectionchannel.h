/*
 * Intel License
 */

#ifndef WOOGEEN_P2P_P2PPEERCONNECTIONCHANNEL_H_
#define WOOGEEN_P2P_P2PPEERCONNECTIONCHANNEL_H_

#include "talk/woogeen/sdk/base/signalingsenderinterface.h"
#include "talk/woogeen/sdk/base/signalingreceiverinterface.h"
#include "talk/woogeen/sdk/base/peerconnectiondependencyfactory.h"
#include "webrtc/base/json.h"

namespace woogeen {

using webrtc::PeerConnectionInterface;

// P2PPeerConnectionChannel callback interface.
// Usually, PeerClient should implement these methods and notify application.
class P2PPeerConnectionChannelObserver {
  public:
    // Triggered when received an invitation.
    virtual void OnInvited(std::string remote_id) = 0;
    // Triggered when remote user accepted the invitation.
    virtual void OnAccepted(std::string remote_id) = 0;
    // Triggered when the WebRTC session is ended.
    virtual void OnStopped(std::string remote_id) = 0;
};

// An instance of P2PPeerConnectionChannel manages a session for a specified remote client.
class P2PPeerConnectionChannel : public SignalingReceiverInterface,
                                 public webrtc::PeerConnectionObserver {
  public:
    explicit P2PPeerConnectionChannel(const std::string& remote_id, SignalingSenderInterface* sender);
    void AddObserver(P2PPeerConnectionChannelObserver* observer);
    void RemoveObserver(P2PPeerConnectionChannelObserver *observer);
    void OnIncomingMessage(const std::string& message) override;
    void Invite(std::function<void()> success, std::function<void(int)> failure);

  protected:
    bool InitializePeerConnection();
    void CreateOffer();

    // Received messages from remote client.
    void OnMessageInvitation();
    void OnMessageAcceptance();
    void OnMessageStop();
    void OnMessageSignal(Json::Value& signal);

    // PeerConnectionObserver
    virtual void OnSignalingChange(PeerConnectionInterface::SignalingState new_state);
    virtual void OnAddStream(MediaStreamInterface* stream);
    virtual void OnRemoveStream(MediaStreamInterface* stream);
    virtual void OnDataChannel(webrtc::DataChannelInterface* data_channel);
    virtual void OnRenegotiationNeeded();
    virtual void OnIceConnectionChange(PeerConnectionInterface::IceConnectionState new_state);
    virtual void OnIceGatheringChange(PeerConnectionInterface::IceGatheringState new_state);
    virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate);

    // CreateSessionDescriptionObserver
    virtual void OnCreateSessionDescriptionSuccess(webrtc::SessionDescriptionInterface* desc);
    virtual void OnCreateSessionDescriptionFailure(const std::string& error);

    // SetSessionDescriptionObserver
    virtual void OnSetSessionDescriptionSuccess();
    virtual void OnSetSessionDescriptionFailure(const std::string& error);

    enum State : int;

  private:
    void ChangeState(State state);
    void SendSignalingMessage(const Json::Value& data, std::function<void()> success, std::function<void(int)> failure);

    SignalingSenderInterface* signaling_sender_;
    std::string remote_id_;
    State state_;
    std::vector<P2PPeerConnectionChannelObserver*> observers_;
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
    // |factory_| is got from PeerConnectionDependencyFactory::Get() which is shared among all PeerConnectionChannels.
    rtc::scoped_refptr<woogeen::PeerConnectionDependencyFactory> factory_;
    Thread* pc_thread_;  // All operations on PeerConnection will be performed on this thread.
    Thread* callback_thread_;  // All callbacks will be executed on this thread.
};

}

#endif // WOOGEEN_P2P_P2PPEERCONNECTIONCHANNEL_H_
