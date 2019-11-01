// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_CONFERENCE_CONFERENCEPEERCONNECTIONCHANNEL_H_
#define OWT_CONFERENCE_CONFERENCEPEERCONNECTIONCHANNEL_H_
#include <memory>
#include <mutex>
#include <unordered_map>
#include <chrono>
#include <random>
#include "talk/owt/sdk/base/peerconnectionchannel.h"
#include "talk/owt/sdk/conference/conferencesocketsignalingchannel.h"
#include "talk/owt/sdk/include/cpp/owt/base/stream.h"
#include "talk/owt/sdk/include/cpp/owt/conference/subscribeoptions.h"
#include "talk/owt/sdk/include/cpp/owt/conference/conferencepublication.h"
namespace owt {
namespace conference {
using namespace owt::base;
// An instance of ConferencePeerConnectionChannel manages a PeerConnection with
// MCU as well as it's signaling through Socket.IO.
class ConferencePeerConnectionChannel
    : public PeerConnectionChannel,
      public std::enable_shared_from_this<ConferencePeerConnectionChannel> {
 public:
  explicit ConferencePeerConnectionChannel(
      PeerConnectionChannelConfiguration& configuration,
      std::shared_ptr<ConferenceSocketSignalingChannel> signaling_channel,
      std::shared_ptr<rtc::TaskQueue> event_queue);
  ~ConferencePeerConnectionChannel();
  // Add a ConferencePeerConnectionChannel observer so it will be notified when
  // this object have some events.
  void AddObserver(ConferencePeerConnectionChannelObserver& observer);
  // Remove a ConferencePeerConnectionChannel observer. If the observer doesn't
  // exist, it will do nothing.
  void RemoveObserver(ConferencePeerConnectionChannelObserver& observer);
  // Publish a local stream to the conference.
  void Publish(
      std::shared_ptr<LocalStream> stream,
      std::function<void(std::string)> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  // Unpublish a local stream to the conference.
  void Unpublish(
      const std::string& session_id,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  // Subscribe a stream from the conference.
  void Subscribe(
      std::shared_ptr<RemoteStream> stream,
      const SubscribeOptions& options,
      std::function<void(std::string)> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  // Unsubscribe a remote stream from the conference.
  void Unsubscribe(
      const std::string& session_id,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  // Continue to transmit specified stream's audio data.
  // If |stream| is a remote stream, MCU will continue to send audio data to
  // client. If |stream| is a local stream, client will continue to send audio
  // data to MCU. This method is expected to be called after |DisableAudio|.
  void PlayAudio(
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  // Stop transmitting specified stream's audio data.
  // If |stream| is a remote stream, MCU will stop sending audio data to client.
  // If |stream| is a local stream, client will stop sending audio data to MCU.
  void PauseAudio(
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  // Continue to transmit specified stream's video data.
  // If |stream| is a remote stream, MCU will continue to send video data to
  // client. If |stream| is a local stream, client will continue to send video
  // data to MCU. This method is expected to be called after |DisableVideo|.
  void PlayVideo(
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  // Stop transmitting specified stream's video data.
  // If |stream| is a remote stream, MCU will stop sending video data to client.
  // If |stream| is a local stream, client will stop sending video data to MCU.
  void PauseVideo(
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  void PlayAudioVideo(
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  void PauseAudioVideo(
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  // Stop current WebRTC session.
  void Stop(
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  // Initialize an ICE restarat.
  void IceRestart();
  // Get the associated stream id if it is a subscription channel.
  std::string GetSubStreamId();
  // Set stream's session ID. This ID is returned by MCU per publish/subscribe.
  void SetSessionId(const std::string& id);
  // Get published or subscribed stream's publicationID or subcriptionID.
  std::string GetSessionId() const;
  // Socket.IO event
  virtual void OnSignalingMessage(sio::message::ptr message);
  // Get statistowt data for the specific stream.
  void GetConnectionStats(
      std::function<void(std::shared_ptr<ConnectionStats>)> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  void GetStats(
      std::function<void(const webrtc::StatsReports& reports)> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  // Called when MCU reports stream/connection is failed or ICE failed.
  void OnStreamError(const std::string& error_message);
 protected:
  void CreateOffer() override;
  void CreateAnswer() override;
  // PeerConnectionObserver
  virtual void OnSignalingChange(
      PeerConnectionInterface::SignalingState new_state) override;
  virtual void OnAddStream(
      rtc::scoped_refptr<MediaStreamInterface> stream) override;
  virtual void OnRemoveStream(
      rtc::scoped_refptr<MediaStreamInterface> stream) override;
  virtual void OnDataChannel(
      rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) override;
  virtual void OnRenegotiationNeeded() override;
  virtual void OnIceConnectionChange(
      PeerConnectionInterface::IceConnectionState new_state) override;
  virtual void OnIceGatheringChange(
      PeerConnectionInterface::IceGatheringState new_state) override;
  virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
  virtual void OnIceCandidatesRemoved(
      const std::vector<cricket::Candidate>& candidates) override;
  // CreateSessionDescriptionObserver
  virtual void OnCreateSessionDescriptionSuccess(
      webrtc::SessionDescriptionInterface* desc) override;
  virtual void OnCreateSessionDescriptionFailure(const std::string& error) override;
  // SetSessionDescriptionObserver
  virtual void OnSetLocalSessionDescriptionSuccess() override;
  virtual void OnSetLocalSessionDescriptionFailure(const std::string& error) override;
  virtual void OnSetRemoteSessionDescriptionSuccess() override;
  virtual void OnSetRemoteSessionDescriptionFailure(const std::string& error) override;
  virtual void OnNetworksChanged() override;
  enum SessionState : int;
  enum NegotiationState : int;
 private:
  // Publish and/or unpublish all streams in pending stream list.
  void ClosePeerConnection();  // Stop session and clean up.
  // Returns true if |pointer| is not nullptr. Otherwise, return false and
  // execute |on_failure|.
  bool CheckNullPointer(
      uintptr_t pointer,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  void SetRemoteDescription(const std::string& type, const std::string& sdp);
  void SendStreamControlMessage(
      const std::string& in_action,
      const std::string& out_action,
      const std::string& operation,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure)
      const;
  void DrainIceCandidates();
  void DoIceRestart();
  void SendPublishMessage(
    sio::message::ptr options,
    std::shared_ptr<LocalStream> stream,
    std::function<void(std::unique_ptr<Exception>)> on_failure);
  std::function<void()> RunInEventQueue(std::function<void()> func);
  // Set publish_success_callback_, subscribe_success_callback_ and
  // failure_callback_ to nullptr.
  void ResetCallbacks();
  bool IsMediaStreamEnded(MediaStreamInterface* stream) const;
  std::shared_ptr<ConferenceSocketSignalingChannel> signaling_channel_;
  std::string session_id_;   //session ID is 1:1 mapping to the subscribed/published stream.
  webrtc::PeerConnectionInterface::SignalingState signaling_state_;
  // If this pc is used for publishing, |local_stream_| will be the stream to be
  // published.
  // Otherwise, |remote_stream_| will be the stream to be subscribed.
  std::shared_ptr<RemoteStream> subscribed_stream_;
  std::shared_ptr<LocalStream> published_stream_;
  // Callbacks for publish or subscribe.
  std::function<void(std::string)> publish_success_callback_;
  std::function<void(std::string)> subscribe_success_callback_;
  std::function<void(std::unique_ptr<Exception>)> failure_callback_;
  std::mutex callback_mutex_;
  // Stored candidates, will be send out after setting remote description.
  // Use sio::message::ptr instead of IceCandidateInterface* to avoid one more
  // deep copy.
  std::vector<sio::message::ptr> ice_candidates_;
  std::mutex candidates_mutex_;
  bool ice_restart_needed_;
  std::mutex observers_mutex_;
  std::vector<std::reference_wrapper<ConferencePeerConnectionChannelObserver>>
      observers_;
  bool connected_;
  // Mutex for firing subscription succeed callback.
  std::mutex sub_stream_added_mutex_;
  bool sub_stream_added_;
  bool sub_server_ready_;
  // Queue for callbacks and events.
  std::shared_ptr<rtc::TaskQueue> event_queue_;
};
}
}
#endif  // OWT_CONFERENCE_CONFERENCEPEERCONNECTIONCHANNEL_H_
