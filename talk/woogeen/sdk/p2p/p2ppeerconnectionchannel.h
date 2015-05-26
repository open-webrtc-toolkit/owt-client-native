/*
 * Intel License
 */

#ifndef WOOGEEN_P2P_P2PPEERCONNECTIONCHANNEL_H_
#define WOOGEEN_P2P_P2PPEERCONNECTIONCHANNEL_H_

#include <memory>
#include <unordered_map>
#include <chrono>
#include "talk/woogeen/sdk/base/signalingsenderinterface.h"
#include "talk/woogeen/sdk/base/signalingreceiverinterface.h"
#include "talk/woogeen/sdk/base/peerconnectiondependencyfactory.h"
#include "talk/woogeen/sdk/base/mediaconstraintsimpl.h"
#include "talk/woogeen/sdk/base/stream.h"
#include "talk/woogeen/sdk/base/peerconnectionchannel.h"
#include "talk/woogeen/sdk/p2p/p2pexception.h"
#include "webrtc/base/json.h"
#include "webrtc/base/messagehandler.h"

namespace woogeen {

// P2PPeerConnectionChannel callback interface.
// Usually, PeerClient should implement these methods and notify application.
class P2PPeerConnectionChannelObserver {
  public:
    // Triggered when received an invitation.
    virtual void OnInvited(const std::string& remote_id) = 0;
    // Triggered when remote user accepted the invitation.
    virtual void OnAccepted(const std::string& remote_id) = 0;
    // Triggered when the WebRTC session is ended.
    virtual void OnStopped(const std::string& remote_id) = 0;
    // Triggered when remote user denied the invitation.
    virtual void OnDenied(const std::string& remote_id) = 0;
    // Triggered when a new stream is added.
    virtual void OnStreamAdded(std::shared_ptr<woogeen::RemoteStream> stream) = 0;
    // Triggered when a remote stream is removed.
    virtual void OnStreamRemoved(std::shared_ptr<woogeen::RemoteStream> stream) = 0;
};

// An instance of P2PPeerConnectionChannel manages a session for a specified remote client.
class P2PPeerConnectionChannel : public SignalingReceiverInterface,
                                 public PeerConnectionChannel {
  public:
    explicit P2PPeerConnectionChannel(const std::string& local_id, const std::string& remote_id, SignalingSenderInterface* sender);
    // Add a P2PPeerConnectionChannel observer so it will be notified when this object have some events.
    void AddObserver(P2PPeerConnectionChannelObserver* observer);
    // Remove a P2PPeerConnectionChannel observer. If the observer doesn't exist, it will do nothing.
    void RemoveObserver(P2PPeerConnectionChannelObserver *observer);
    // Implementation of SignalingReceiverInterface. Handle signaling message received from remote side.
    void OnIncomingSignalingMessage(const std::string& message) override;
    // Invite a remote client to start a WebRTC session.
    void Invite(std::function<void()> on_success, std::function<void(std::unique_ptr<P2PException>)> on_failure);
    // Accept a remote client's invitation.
    void Accept(std::function<void()> on_success, std::function<void(std::unique_ptr<P2PException>)> on_failure);
    // Deny a remote client's invitation.
    void Deny(std::function<void()> on_success, std::function<void(std::unique_ptr<P2PException>)> on_failure);
    // Publish a local stream to remote user.
    void Publish(std::shared_ptr<LocalStream> stream, std::function<void()> on_success, std::function<void(std::unique_ptr<P2PException>)> on_failure);
    // Unpublish a local stream to remote user.
    void Unpublish(std::shared_ptr<LocalStream> stream, std::function<void()> on_success, std::function<void(std::unique_ptr<P2PException>)> on_failure);
    // Stop current WebRTC session.
    void Stop(std::function<void()> on_success, std::function<void(std::unique_ptr<P2PException>)> on_failure);

  protected:
    void CreateOffer();
    void CreateAnswer();

    // Received messages from remote client.
    void OnMessageInvitation();
    void OnMessageAcceptance();
    void OnMessageStop();
    void OnMessageDeny();
    void OnMessageSignal(Json::Value& signal);
    void OnMessageNegotiationNeeded();
    void OnMessageNegotiationAcceptance();
    void OnMessageStreamType(Json::Value& type_info);

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
    virtual void OnSetLocalSessionDescriptionSuccess();
    virtual void OnSetLocalSessionDescriptionFailure(const std::string& error);
    virtual void OnSetRemoteSessionDescriptionSuccess();
    virtual void OnSetRemoteSessionDescriptionFailure(const std::string& error);

    enum SessionState : int;
    enum NegotiationState : int;

  private:
    void ChangeSessionState(SessionState state);
    void ChangeNegotiationState(NegotiationState state);
    void SendSignalingMessage(const Json::Value& data, std::function<void()> success, std::function<void(std::unique_ptr<P2PException>)> failure);
    // Publish and/or unpublish all streams in pending stream list.
    void DrainPendingStreams();
    void CheckWaitedList();  // Check pending streams and negotiation requests.
    void SendNegotiationAccepted();
    void SendAcceptance(std::function<void()> on_success, std::function<void(std::unique_ptr<P2PException>)> on_failure);
    void SendStop(std::function<void()> on_success, std::function<void(std::unique_ptr<P2PException>)> on_failure);
    void SendDeny(std::function<void()> on_success, std::function<void(std::unique_ptr<P2PException>)> on_failure);
    void ClosePeerConnection();  // Stop session and clean up.
    // Returns true if |pointer| is not nullptr. Otherwise, return false and execute |on_failure|.
    bool CheckNullPointer(uintptr_t pointer, std::function<void(std::unique_ptr<P2PException>)>on_failure);

    SignalingSenderInterface* signaling_sender_;
    std::string local_id_;
    std::string remote_id_;
    SessionState session_state_;
    NegotiationState negotiation_state_;
    bool negotiation_needed_;  // Indicates if negotiation needed event is triggered but haven't send out negotiation request.
    std::unordered_map<std::string, std::string> remote_stream_type_;  // Key is remote media stream's label, value is type.
    std::vector<std::shared_ptr<LocalStream>> pending_publish_streams_;  // Streams need to be published.
    std::vector<std::shared_ptr<LocalStream>> pending_unpublish_streams_;  // Streams need to be unpublished.
    std::mutex pending_publish_streams_mutex_;
    std::mutex pending_unpublish_streams_mutex_;
    std::vector<P2PPeerConnectionChannelObserver*> observers_;
    std::chrono::time_point<std::chrono::system_clock> last_disconnect_;  // Last time |peer_connection_| changes its state to "disconnect"
    Thread* callback_thread_;  // All callbacks will be executed on this thread.
};

}

#endif // WOOGEEN_P2P_P2PPEERCONNECTIONCHANNEL_H_
