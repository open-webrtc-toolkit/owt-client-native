// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef WOOGEEN_P2P_P2PPEERCONNECTIONCHANNEL_H_
#define WOOGEEN_P2P_P2PPEERCONNECTIONCHANNEL_H_
#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include "talk/owt/sdk/base/peerconnectiondependencyfactory.h"
#include "talk/owt/sdk/base/peerconnectionchannel.h"
#include "talk/owt/sdk/include/cpp/owt/base/stream.h"
#include "talk/owt/sdk/include/cpp/owt/base/exception.h"
#include "talk/owt/sdk/include/cpp/owt/p2p/p2psignalingsenderinterface.h"
#include "talk/owt/sdk/include/cpp/owt/p2p/p2psignalingreceiverinterface.h"
#include "webrtc/sdk/media_constraints.h"
#include "webrtc/rtc_base/strings/json.h"
#include "webrtc/rtc_base/message_handler.h"
#include "webrtc/rtc_base/task_queue.h"
#include "webrtc/rtc_base/thread_annotations.h"
namespace owt {
namespace p2p {
using namespace owt::base;
// P2PPeerConnectionChannel callback interface.
// Usually, PeerClient should implement these methods and notify application.
class P2PPeerConnectionChannelObserver {
 public:
  // Triggered when remote user send data via data channel.
  // Currently, data is string.
  virtual void OnMessageReceived(const std::string& remote_id,
                                 const std::string& message) = 0;
  // Triggered when a new stream is added.
  virtual void OnStreamAdded(
      std::shared_ptr<RemoteStream> stream) = 0;
  // Triggered when the WebRTC session is ended.
  virtual void OnStopped(const std::string& remote_id) = 0;
};
// An instance of P2PPeerConnectionChannel manages a session for a specified
// remote client.
class P2PPeerConnectionChannel : public P2PSignalingReceiverInterface,
                                 public PeerConnectionChannel,
                                 public std::enable_shared_from_this<P2PPeerConnectionChannel> {
 public:
  explicit P2PPeerConnectionChannel(
      PeerConnectionChannelConfiguration configuration,
      const std::string& local_id,
      const std::string& remote_id,
      P2PSignalingSenderInterface* sender,
      std::shared_ptr<rtc::TaskQueue> event_queue);
  // If event_queue is not provided, a new event queue will be used. That means,
  // a new thread will be created for each P2PPeerConnection. Currently, iOS
  // SDK's RTCP2PPeerConnection is a pure Obj-C file, so it does not maintain
  // event queue.
  explicit P2PPeerConnectionChannel(
      PeerConnectionChannelConfiguration configuration,
      const std::string& local_id,
      const std::string& remote_id,
      P2PSignalingSenderInterface* sender);
  virtual ~P2PPeerConnectionChannel();
  // Add a P2PPeerConnectionChannel observer so it will be notified when this
  // object have some events.
  void AddObserver(P2PPeerConnectionChannelObserver* observer);
  // Remove a P2PPeerConnectionChannel observer. If the observer doesn't exist,
  // it will do nothing.
  void RemoveObserver(P2PPeerConnectionChannelObserver* observer);
  // Implementation of P2PSignalingReceiverInterface. Handle signaling message
  // received from remote side.
  void OnIncomingSignalingMessage(const std::string& message) override;
  // Publish a local stream to remote user.
  void Publish(std::shared_ptr<LocalStream> stream,
               std::function<void()> on_success,
               std::function<void(std::unique_ptr<Exception>)> on_failure);
  // Unpublish a local stream to remote user.
  void Unpublish(std::shared_ptr<LocalStream> stream,
                 std::function<void()> on_success,
                 std::function<void(std::unique_ptr<Exception>)> on_failure);
  // Send message to remote user.
  void Send(const std::string& message,
            std::function<void()> on_success,
            std::function<void(std::unique_ptr<Exception>)> on_failure);
  // Stop current WebRTC session.
  void Stop(std::function<void()> on_success,
            std::function<void(std::unique_ptr<Exception>)> on_failure);
  // Deprecated. Get statistics data for the specific connection.
  void GetConnectionStats(
      std::function<void(std::shared_ptr<ConnectionStats>)> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  // Get statistics data for the specific connection.
  void GetConnectionStats(
      std::function<void(std::shared_ptr<RTCStatsReport>)> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  void GetStats(
      std::function<void(const webrtc::StatsReports& reports)> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  bool HaveLocalOffer();
  std::shared_ptr<LocalStream> GetLatestLocalStream();
  std::function<void()> GetLatestPublishSuccessCallback();
  std::function<void(std::unique_ptr<Exception>)> GetLatestPublishFailureCallback();
  bool IsAbandoned();
  bool IsStale();
 protected:
  void CreateOffer() override;
  void CreateAnswer() override;
  void OnNegotiationNeeded();
  // Received messages from remote client.
  void OnMessageUserAgent(Json::Value& ua);
  void OnMessageStop();
  void OnMessageSignal(Json::Value& signal);
  void OnMessageTrackSources(Json::Value& track_sources);
  void OnMessageStreamInfo(Json::Value& stream_info);
  void OnMessageTracksAdded(Json::Value& stream_tracks);
  void OnMessageDataReceived(std::string& id);
  // PeerConnectionObserver
  virtual void OnSignalingChange(
      PeerConnectionInterface::SignalingState new_state) override;
  virtual void OnAddStream(rtc::scoped_refptr<MediaStreamInterface> stream) override;
  virtual void OnRemoveStream(rtc::scoped_refptr<MediaStreamInterface> stream) override;
  virtual void OnDataChannel(
      rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) override;
  virtual void OnIceConnectionChange(
      PeerConnectionInterface::IceConnectionState new_state) override;
  virtual void OnIceGatheringChange(
      PeerConnectionInterface::IceGatheringState new_state) override;
  virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
  virtual void OnRenegotiationNeeded() override;
  // DataChannelObserver
  virtual void OnDataChannelStateChange() override;
  virtual void OnDataChannelMessage(const webrtc::DataBuffer& buffer) override;
  // CreateSessionDescriptionObserver
  virtual void OnCreateSessionDescriptionSuccess(
      webrtc::SessionDescriptionInterface* desc) override;
  virtual void OnCreateSessionDescriptionFailure(const std::string& error) override;
  // SetSessionDescriptionObserver
  virtual void OnSetLocalSessionDescriptionSuccess() override;
  virtual void OnSetLocalSessionDescriptionFailure(const std::string& error) override;
  virtual void OnSetRemoteSessionDescriptionSuccess() override;
  virtual void OnSetRemoteSessionDescriptionFailure(const std::string& error) override;
  enum SessionState : int;
  enum NegotiationState : int;
 private:
  void ChangeSessionState(SessionState state);
  void SendSignalingMessage(
      const Json::Value& data,
      std::function<void()> success = nullptr,
      std::function<void(std::unique_ptr<Exception>)> failure = nullptr);
  // Publish and/or unpublish all streams in pending stream list.
  void DrainPendingStreams();
  void TriggerOnStopped();
  void CheckWaitedList();  // Check pending streams and negotiation requests.
  void SendStop(std::function<void()> on_success,
                std::function<void(std::unique_ptr<Exception>)> on_failure);
  // Returns a new reference, so lifetime of connection lasts at least until end of caller.
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> GetPeerConnectionRef();
  void ClosePeerConnection();  // Stop session and clean up.
  // Returns true if |pointer| is not nullptr. Otherwise, return false and
  // execute |on_failure|.
  bool CheckNullPointer(
      uintptr_t pointer,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  webrtc::DataBuffer CreateDataBuffer(const std::string& data);
  void CreateDataChannel(const std::string& label);
  // Send all messages in pending message list.
  void DrainPendingMessages();
  // Process any waiting ICE Candidates
  void DrainPendingRemoteCandidates();
  // Cleans all variables associated with last peerconnection.
  void CleanLastPeerConnection();
  // Returns user agent info as JSON object.
  Json::Value UaInfo();
  // Set remote capability flags based on UA.
  void HandleRemoteCapability(Json::Value& ua);
  void SendUaInfo();
  void ClearPendingStreams();
  rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel_;
  P2PSignalingSenderInterface* signaling_sender_;
  std::string local_id_;
  std::string remote_id_;
  SessionState session_state_;
  // Indicates if negotiation needed event is triggered or received negotiation
  // request from remote side, but haven't send out offer.
  bool negotiation_needed_;
  // Key is remote media stream's track id, value is type ("mic", "camera",
  // "screen-cast").
  rtc::CriticalSection remote_track_source_info_crit_;
  std::unordered_map<std::string, std::string> remote_track_source_info_
      RTC_GUARDED_BY(remote_track_source_info_crit_);
  // Key is local media stream's track id, value is media stream's label.
  std::unordered_map<std::string, std::string> local_stream_tracks_info_;
  std::mutex local_stream_tracks_info_mutex_;
  // Key is remote media stream's label, value is RemoteStream instance.
  std::unordered_map<std::string, std::shared_ptr<RemoteStream>> remote_streams_;
  // Streams need to be published.
  std::vector<std::shared_ptr<LocalStream>> pending_publish_streams_;
  // Streams need to be unpublished.
  std::vector<std::shared_ptr<LocalStream>> pending_unpublish_streams_;
  // A set of labels for streams published to remote side.
  // |Publish| adds its argument to this vector, |Unpublish| removes it.
  std::unordered_set<std::string> published_streams_;
  // A set of labels for streams are publishing to remote side.
  std::unordered_set<std::string> publishing_streams_;
  std::mutex pending_publish_streams_mutex_;
  std::mutex pending_unpublish_streams_mutex_;
  // Shared by |published_streams_| and |publishing_streams_|.
  std::mutex published_streams_mutex_;
  std::vector<P2PPeerConnectionChannelObserver*> observers_;
  std::unordered_map<std::string, std::function<void()>> publish_success_callbacks_;
  // Store remote SDP if it cannot be set currently.
  std::unique_ptr<webrtc::SessionDescriptionInterface> pending_remote_sdp_;
  std::mutex last_disconnect_mutex_;
  std::chrono::time_point<std::chrono::system_clock>
      last_disconnect_;  // Last time |peer_connection_| changes its state to
                         // "disconnect".
  int reconnect_timeout_;  // Unit: second.
  int message_seq_num_; // Message ID to be sent through data channel.
  // Messages need to be sent once data channel is ready.
  std::vector<std::string> pending_messages_;
  // Protects |pending_messages_|.
  std::mutex pending_messages_mutex_;
  // Protects |ended_|
  std::mutex ended_mutex_;
  // Hold incoming ICE candidates if remote session description not yet processed.
  rtc::CriticalSection pending_remote_candidates_crit_;
  std::vector<std::unique_ptr<webrtc::IceCandidateInterface>>
      pending_remote_candidates_ RTC_GUARDED_BY(pending_remote_candidates_crit_);
  // Indicates whether remote client supports WebRTC Plan B
  // (https://tools.ietf.org/html/draft-uberti-rtcweb-plan-00).
  // If plan B is not supported, at most one audio/video track is supported.
  bool remote_side_supports_plan_b_;
  bool remote_side_supports_remove_stream_;
  bool remote_side_supports_unified_plan_;
  bool is_creating_offer_;  // It will be true during creating and setting offer.
  bool remote_side_supports_continual_ice_gathering_;
  // Removing ack for data channel messages. If
  // |remote_side_ignores_datachannel_ack_| is true, don't send acks.
  // https://github.com/open-webrtc-toolkit/owt-server-p2p/issues/17.
  bool remote_side_ignores_datachannel_acks_;
  std::mutex is_creating_offer_mutex_;
  // Queue for callbacks and events.
  std::shared_ptr<rtc::TaskQueue> event_queue_;
  std::shared_ptr<LocalStream> latest_local_stream_;
  std::function<void()> latest_publish_success_callback_;
  std::function<void(std::unique_ptr<Exception>)> latest_publish_failure_callback_;
  bool ua_sent_;
  std::mutex stop_send_mutex_;
  bool stop_send_needed_;
  bool remote_side_offline_;
  bool ended_;
  std::chrono::time_point<std::chrono::system_clock> created_time_;
};
}
}
#endif  // WOOGEEN_P2P_P2PPEERCONNECTIONCHANNEL_H_
