/*
 * Intel License
 */

#ifndef WOOGEEN_CONFERENCE_CONFERENCEPEERCONNECTIONCHANNEL_H_
#define WOOGEEN_CONFERENCE_CONFERENCEPEERCONNECTIONCHANNEL_H_

#include <memory>
#include <unordered_map>
#include <chrono>
#include <random>
#include "talk/woogeen/sdk/base/peerconnectiondependencyfactory.h"
#include "talk/woogeen/sdk/base/mediaconstraintsimpl.h"
#include "talk/woogeen/sdk/base/stream.h"
#include "talk/woogeen/sdk/base/peerconnectionchannel.h"
#include "talk/woogeen/sdk/conference/conferenceexception.h"
#include "talk/woogeen/sdk/conference/conferencesignalingchannelinterface.h"
#include "webrtc/base/json.h"
#include "webrtc/base/messagehandler.h"

namespace woogeen {

// ConferencePeerConnectionChannel callback interface.
// Usually, ConferenceClient should implement these methods and notify application.
// TODO: Modify this for conference.
class ConferencePeerConnectionChannelObserver {
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
    virtual void OnStreamAdded(woogeen::RemoteStream* stream) = 0;
    // Triggered when a remote stream is removed.
    virtual void OnStreamRemoved(woogeen::RemoteStream* stream) = 0;
};

// An instance of ConferencePeerConnectionChannel manages a PeerConnection with MCU.
class ConferencePeerConnectionChannel : public PeerConnectionChannel {
  public:
    explicit ConferencePeerConnectionChannel(PeerConnectionChannelConfiguration& configuration, std::shared_ptr<ConferenceSignalingChannelInterface> signaling_channel);
    ~ConferencePeerConnectionChannel();
    // Add a ConferencePeerConnectionChannel observer so it will be notified when this object have some events.
    void AddObserver(ConferencePeerConnectionChannelObserver* observer);
    // Remove a ConferencePeerConnectionChannel observer. If the observer doesn't exist, it will do nothing.
    void RemoveObserver(ConferencePeerConnectionChannelObserver *observer);
    // Publish a local stream to the conference.
    void Publish(std::shared_ptr<LocalStream> stream, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
    // Unpublish a local stream to the conference.
    void Unpublish(std::shared_ptr<LocalStream> stream, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
    // Subscribe a stream from the conference.
    void Subscribe(std::shared_ptr<RemoteStream> stream, std::function<void(std::shared_ptr<RemoteStream> stream)> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
    // Unsubscribe a remote stream from the conference.
    void Unsubscribe(std::shared_ptr<RemoteStream> stream, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
    // Continue to transmit specified stream's audio data.
    // If |stream| is a remote stream, MCU will continue to send audio data to client. If |stream| is a local stream, client will continue to send audio data to MCU. This method is expected to be called after |DisableAudio|.
    void PlayAudio(std::shared_ptr<Stream> stream, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
    // Stop transmitting specified stream's audio data.
    // If |stream| is a remote stream, MCU will stop sending audio data to client. If |stream| is a local stream, client will stop sending audio data to MCU.
    void PauseAudio(std::shared_ptr<Stream> stream, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
    // Continue to transmit specified stream's video data.
    // If |stream| is a remote stream, MCU will continue to send video data to client. If |stream| is a local stream, client will continue to send video data to MCU. This method is expected to be called after |DisableVideo|.
    void PlayVideo(std::shared_ptr<Stream> stream, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
    // Stop transmitting specified stream's video data.
    // If |stream| is a remote stream, MCU will stop sending video data to client. If |stream| is a local stream, client will stop sending video data to MCU.
    void PauseVideo(std::shared_ptr<Stream> stream, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
    // Stop current WebRTC session.
    void Stop(std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure);

  protected:
    void CreateOffer();
    void CreateAnswer();

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
    void SendSignalingMessage(const Json::Value& data, std::function<void()> success, std::function<void(std::unique_ptr<ConferenceException>)> failure);
    // Publish and/or unpublish all streams in pending stream list.
    void CheckWaitedList();  // Check pending streams and negotiation requests.
    void ClosePeerConnection();  // Stop session and clean up.
    // Returns true if |pointer| is not nullptr. Otherwise, return false and execute |on_failure|.
    bool CheckNullPointer(uintptr_t pointer, std::function<void(std::unique_ptr<ConferenceException>)>on_failure);
    int RandomInt(int lower_bound, int upper_bound);
    void SetRemoteDescription(const std::string& type, const std::string& sdp);
    void SendStreamControlMessage(const std::shared_ptr<Stream> stream, const std::string& in_action, const std::string& out_action, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure) const;

    std::shared_ptr<ConferenceSignalingChannelInterface> signaling_channel_;
    int session_id_;
    int message_seq_;

    // If this pc is used for publishing, |local_stream_| will be the stream to be published.
    // Otherwise, |remote_stream_| will be the stream to be subscribed.
    std::shared_ptr<RemoteStream> subscribed_stream_;
    std::shared_ptr<LocalStream> published_stream_;

    // Callbacks for publish or subscribe.
    std::function<void(std::shared_ptr<RemoteStream> stream)> subscribe_success_callback_;
    std::function<void()> publish_success_callback_;
    std::function<void(std::unique_ptr<ConferenceException>)> failure_callback_;

    SessionState session_state_;
    NegotiationState negotiation_state_;
    std::vector<ConferencePeerConnectionChannelObserver*> observers_;
    Thread* callback_thread_;  // All callbacks will be executed on this thread.
};

};

#endif // WOOGEEN_CONFERENCE_CONFERENCEPEERCONNECTIONCHANNEL_H_
