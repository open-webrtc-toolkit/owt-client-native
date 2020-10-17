// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include <thread>
#include <vector>
#include <iostream>
#include "talk/owt/sdk/base/eventtrigger.h"
#include "talk/owt/sdk/base/functionalobserver.h"
#include "talk/owt/sdk/base/sdputils.h"
#include "talk/owt/sdk/base/sysinfo.h"
#include "talk/owt/sdk/p2p/p2ppeerconnectionchannel.h"
#include "webrtc/rtc_base/logging.h"
#include "webrtc/api/task_queue/default_task_queue_factory.h"

using namespace rtc;
namespace owt {
namespace p2p {
using std::string;
enum P2PPeerConnectionChannel::SessionState : int {
  kSessionStateReady = 1,   // Indicate the channel is ready. This is the initial state.
  kSessionStateOffered,     // Indicates local client has sent a user agent and
                            // waiting for an remote SDP.
  kSessionStatePending,     // Indicates local client received an user agent and
                            // waiting for user's response.
  kSessionStateMatched,     // Indicates both sides agreed to start a WebRTC
                            // session. One of them will send an offer soon.
  kSessionStateConnecting,  // Indicates both sides are trying to connect to the
                            // other side.
  kSessionStateConnected,   // Indicates PeerConnection has been established.
};
// Signaling message type
const string kMessageTypeKey = "type";
const string kMessageDataKey = "data";
const string kChatClosed = "chat-closed";
const string kChatSignal = "chat-signal";
const string kChatTrackSources = "chat-track-sources";
const string kChatStreamInfo = "chat-stream-info";
const string kChatTracksAdded = "chat-tracks-added";
const string kChatTracksRemoved = "chat-tracks-removed";
const string kChatDataReceived = "chat-data-received";
const string kChatUserAgent = "chat-ua";
// Track information member key
const string kTrackIdKey = "id";
const string kTrackSourceKey = "source";
// Stream information member key
const string kStreamIdKey = "id";
const string kStreamTracksKey = "tracks";
const string kStreamAudioSourceKey = "audio";
const string kStreamVideoSourceKey = "video";
const string kStreamSourceKey = "source";
// Session description member key
const string kSessionDescriptionTypeKey = "type";
const string kSessionDescriptionSdpKey = "sdp";
// ICE candidate member key
const string kIceCandidateSdpNameKey = "candidate";
const string kIceCandidateSdpMidKey = "sdpMid";
const string kIceCandidateSdpMLineIndexKey = "sdpMLineIndex";
// UA member keys
// SDK section
const string kUaSdkKey = "sdk";
const string kUaSdkTypeKey = "type";
const string kUaSdkVersionKey = "version";
// Runtime section
const string kUaRuntimeKey = "runtime";
const string kUaRuntimeNameKey = "name";
const string kUaRuntimeVersionKey = "version";
// OS section
const string kUaOsKey = "os";
const string kUaOsNameKey = "name";
const string kUaOsVersionKey = "version";
// Capabilities section
const string kUaCapabilitiesKey = "capabilities";
const string kUaContinualGatheringKey = "continualIceGathering";
const string kUaUnifiedPlanKey = "unifiedPlan";
const string kUaStreamRemovableKey = "streamRemovable";
const string kUaIgnoresDataChannelAcksKey = "ignoreDataChannelAcks";
// Text message sent through data channel
const string kDataChannelLabelForTextMessage = "message";
const string kTextMessageDataKey = "data";
const string kTextMessageIdKey = "id";
const int kStaleThresholdSecs = 300; // 5 min
P2PPeerConnectionChannel::P2PPeerConnectionChannel(
    PeerConnectionChannelConfiguration configuration,
    const std::string& local_id,
    const std::string& remote_id,
    P2PSignalingSenderInterface* sender,
    std::shared_ptr<rtc::TaskQueue> event_queue)
    : PeerConnectionChannel(configuration),
      signaling_sender_(sender),
      local_id_(local_id),
      remote_id_(remote_id),
      session_state_(kSessionStateReady),
      negotiation_needed_(false),
      pending_remote_sdp_(nullptr),
      last_disconnect_(
          std::chrono::time_point<std::chrono::system_clock>::max()),
      reconnect_timeout_(10),
      message_seq_num_(0),
      remote_side_supports_plan_b_(false),
      remote_side_supports_remove_stream_(false),
      remote_side_supports_unified_plan_(true),
      is_creating_offer_(false),
      remote_side_supports_continual_ice_gathering_(true),
      remote_side_ignores_datachannel_acks_(false),
      ua_sent_(false),
      stop_send_needed_(true),
      remote_side_offline_(false),
      ended_(false),
      created_time_(std::chrono::system_clock::now()) {
  RTC_CHECK(signaling_sender_);
  InitializePeerConnection();
  CreateDataChannel(kDataChannelLabelForTextMessage);
  if (event_queue) {
    event_queue_ = event_queue;
  } else {
    auto task_queue_factory_ = webrtc::CreateDefaultTaskQueueFactory();
    event_queue_ =
        std::make_unique<rtc::TaskQueue>(task_queue_factory_->CreateTaskQueue(
            "P2PClientEventQueue", webrtc::TaskQueueFactory::Priority::NORMAL));
  }
}
P2PPeerConnectionChannel::P2PPeerConnectionChannel(
    PeerConnectionChannelConfiguration configuration,
    const std::string& local_id,
    const std::string& remote_id,
    P2PSignalingSenderInterface* sender)
    : P2PPeerConnectionChannel(configuration,
                               local_id,
                               remote_id,
                               sender,
                               nullptr) {}

P2PPeerConnectionChannel::~P2PPeerConnectionChannel() {
  if (signaling_sender_) {
    SendStop(nullptr, nullptr);
    delete signaling_sender_;
  }
  if (peer_connection_ != nullptr) {
    peer_connection_->Close();
  }
}
// Returns a copy of the reference. If the reference is not null, then caller can invoke methods locking
rtc::scoped_refptr<webrtc::PeerConnectionInterface> P2PPeerConnectionChannel::GetPeerConnectionRef() {
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> temp_pc_;
  {
    const std::lock_guard<std::mutex> lock(ended_mutex_);
    if (!ended_) {
      temp_pc_ = peer_connection_;
    }
  }
  return temp_pc_;
}

void P2PPeerConnectionChannel::Publish(
    std::shared_ptr<LocalStream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  RTC_LOG(LS_INFO) << "Publishing a local stream.";
  // Add reference to peer connection until end of function.
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> temp_pc_ = GetPeerConnectionRef();
  if (!CheckNullPointer((uintptr_t)stream.get(), on_failure)) {
    RTC_LOG(LS_INFO) << "Local stream cannot be nullptr.";
    return;
  }
  RTC_CHECK(stream->MediaStream());
  std::string stream_label = stream->MediaStream()->id();
  {
    std::lock_guard<std::mutex> lock(published_streams_mutex_);
    if (published_streams_.find(stream_label) != published_streams_.end() ||
        publishing_streams_.find(stream_label) != publishing_streams_.end()) {
      if (on_failure) {
        std::unique_ptr<Exception> e(
            new Exception(ExceptionType::kP2PClientInvalidArgument,
                          "The stream is already published."));
        on_failure(std::move(e));
      }
      return;
    }
  }
  // Send chat-closed to workaround known browser bugs, together with
  // user agent information once and once only.
  {
    std::lock_guard<std::mutex> lock(stop_send_mutex_);
    if (stop_send_needed_) {
      SendStop(nullptr, nullptr);
      stop_send_needed_ = false;
    }
    if (!ua_sent_) {
      SendUaInfo();
      ua_sent_ = true;
    }
  }
  rtc::scoped_refptr<webrtc::MediaStreamInterface> media_stream =
      stream->MediaStream();
  // Calling [stream|track]->id() runs on signaling_thread, so call outside of locks
  std::string stream_id = media_stream->id();
  std::pair<std::string, std::string> stream_track_info;
  for (const auto& track : media_stream->GetAudioTracks()) {
    std::string track_id = track->id();
    {
      std::lock_guard<std::mutex> lock(local_stream_tracks_info_mutex_);
      if (local_stream_tracks_info_.find(track_id) ==
          local_stream_tracks_info_.end()) {
        stream_track_info = std::make_pair(track_id, stream_id);
        local_stream_tracks_info_.insert(stream_track_info);
      }
    }
  }
  for (const auto& track : media_stream->GetVideoTracks()) {
    std::string track_id = track->id();
    {
      std::lock_guard<std::mutex> lock(local_stream_tracks_info_mutex_);
      if (local_stream_tracks_info_.find(track_id) ==
          local_stream_tracks_info_.end()) {
        stream_track_info = std::make_pair(track_id, stream_id);
        local_stream_tracks_info_.insert(stream_track_info);
      }
    }
  }
  {
    std::lock_guard<std::mutex> lock(pending_publish_streams_mutex_);
    pending_publish_streams_.push_back(stream);
  }
  {
    std::lock_guard<std::mutex> lock(published_streams_mutex_);
    publishing_streams_.insert(stream_label);
  }
  RTC_LOG(LS_INFO) << "Session state: " << session_state_;
  if (on_success) {
    on_success();
  }
  DrainPendingStreams();
}
void P2PPeerConnectionChannel::Send(
    const std::string& message,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  // Send chat-closed to workaround known browser bugs, together with
  // user agent information once and once only.
  {
    std::lock_guard<std::mutex> lock(stop_send_mutex_);
    if (stop_send_needed_) {
      SendStop(nullptr, nullptr);
      stop_send_needed_ = false;
    }
    if (!ua_sent_) {
      SendUaInfo();
      ua_sent_ = true;
    }
  }
  // Try to send the text message.
  Json::Value content;
  if (message_seq_num_ == INT_MAX) {
    message_seq_num_ = 0;
  }
  int message_id = message_seq_num_++;
  content[kTextMessageIdKey] = std::to_string(message_id);
  content[kTextMessageDataKey] = message;
  std::string data = rtc::JsonValueToString(content);
  rtc::scoped_refptr<webrtc::DataChannelInterface> temp_dc_;
  {
    std::lock_guard<std::mutex> lock(ended_mutex_);
    if (!ended_) {
      temp_dc_ = data_channel_;
    }
  }
  if (temp_dc_ && temp_dc_->state() == webrtc::DataChannelInterface::DataState::kOpen) {
    temp_dc_->Send(CreateDataBuffer(data));
  } else {
    {
      std::lock_guard<std::mutex> lock(pending_messages_mutex_);
      pending_messages_.push_back(data);
    }
  }
  if (on_success) {
    on_success();
  }
  (void) on_failure; // UNUSED
  DrainPendingMessages();
}
void P2PPeerConnectionChannel::ChangeSessionState(SessionState state) {
  RTC_LOG(LS_INFO) << "PeerConnectionChannel change session state : " << state;
  session_state_ = state;
}
void P2PPeerConnectionChannel::AddObserver(
    P2PPeerConnectionChannelObserver* observer) {
  observers_.push_back(observer);
}
void P2PPeerConnectionChannel::RemoveObserver(
    P2PPeerConnectionChannelObserver* observer) {
  observers_.erase(std::remove(observers_.begin(), observers_.end(), observer),
                   observers_.end());
}
void P2PPeerConnectionChannel::CreateOffer() {
  // Get reference so peer_connection_ is not deleted before calling CreateOffer.
  {
    std::lock_guard<std::mutex> lock(is_creating_offer_mutex_);
    if (is_creating_offer_) {
      // Store creating offer request.
      negotiation_needed_ = true;
      return;
    } else {
      is_creating_offer_ = true;
    }
  }
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> temp_pc_ = GetPeerConnectionRef();
  RTC_LOG(LS_INFO) << "Create offer.";
  negotiation_needed_ = false;
  scoped_refptr<FunctionalCreateSessionDescriptionObserver> observer =
      FunctionalCreateSessionDescriptionObserver::Create(
          std::bind(
              &P2PPeerConnectionChannel::OnCreateSessionDescriptionSuccess,
              this, std::placeholders::_1),
          std::bind(
              &P2PPeerConnectionChannel::OnCreateSessionDescriptionFailure,
              this, std::placeholders::_1));
  if (temp_pc_) {
    temp_pc_->CreateOffer(
        observer, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
  }
}
void P2PPeerConnectionChannel::CreateAnswer() {
  RTC_LOG(LS_INFO) << "Create answer.";
  // Get reference so peer_connection_ is not deleted before calling CreateAnswer
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> temp_pc_ = GetPeerConnectionRef();
  scoped_refptr<FunctionalCreateSessionDescriptionObserver> observer =
      FunctionalCreateSessionDescriptionObserver::Create(
          std::bind(
              &P2PPeerConnectionChannel::OnCreateSessionDescriptionSuccess,
              this, std::placeholders::_1),
          std::bind(
              &P2PPeerConnectionChannel::OnCreateSessionDescriptionFailure,
              this, std::placeholders::_1));
  if (temp_pc_) {
    temp_pc_->CreateAnswer(
        observer, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
  }
}
void P2PPeerConnectionChannel::SendSignalingMessage(
    const Json::Value& data,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  RTC_CHECK(signaling_sender_);
  std::string json_string = rtc::JsonValueToString(data);
  signaling_sender_->SendSignalingMessage(
      json_string, remote_id_, on_success,
      [=](std::unique_ptr<Exception> exception) {
        if (exception->Type() == ExceptionType::kP2PMessageTargetUnreachable) {
          remote_side_offline_ = true;
          ExceptionType type = exception->Type();
          std::string msg = exception->Message();
        }
        if (on_failure)
          on_failure(std::move(exception));
      });
}
void P2PPeerConnectionChannel::OnIncomingSignalingMessage(
    const std::string& message) {
  RTC_LOG(LS_INFO) << "OnIncomingMessage: " << message;
  RTC_DCHECK(!message.empty());
  Json::Reader reader;
  Json::Value json_message;
  if (!reader.parse(message, json_message)) {
    RTC_LOG(LS_WARNING) << "Cannot parse incoming message.";
    return;
  }
  std::string message_type;
  rtc::GetStringFromJsonObject(json_message, kMessageTypeKey, &message_type);
  if (message_type.empty()) {
    RTC_LOG(LS_WARNING) << "Cannot get type from incoming message.";
    return;
  }
  if (message_type == kChatUserAgent) {
    // Send back user agent info once and only once
    {
      std::lock_guard<std::mutex> lock(stop_send_mutex_);
      if (!ua_sent_) {
        SendUaInfo();
        ua_sent_ = true;
        stop_send_needed_ = false;
      }
    }
    Json::Value ua;
    rtc::GetValueFromJsonObject(json_message, kMessageDataKey, &ua);
    OnMessageUserAgent(ua);
  } else if (message_type == kChatTrackSources) {
    Json::Value track_sources;
    rtc::GetValueFromJsonObject(json_message, kMessageDataKey, &track_sources);
    OnMessageTrackSources(track_sources);
  } else if (message_type == kChatStreamInfo) {
    Json::Value stream_info;
    rtc::GetValueFromJsonObject(json_message, kMessageDataKey, &stream_info);
    OnMessageStreamInfo(stream_info);
  } else if (message_type == kChatSignal) {
    Json::Value signal;
    rtc::GetValueFromJsonObject(json_message, kMessageDataKey, &signal);
    OnMessageSignal(signal);
  } else if (message_type == kChatTracksAdded) {
    Json::Value tracks;
    rtc::GetValueFromJsonObject(json_message, kMessageDataKey, &tracks);
    OnMessageTracksAdded(tracks);
  } else if (message_type == kChatClosed) {
    OnMessageStop();
  } else {
    RTC_LOG(LS_WARNING) << "Received unknown message type : " << message_type;
    return;
  }
}
void P2PPeerConnectionChannel::OnMessageUserAgent(Json::Value& ua) {
  HandleRemoteCapability(ua);
  switch (session_state_) {
    case kSessionStateReady:
    case kSessionStatePending:
      ChangeSessionState(kSessionStateMatched);
      break;
    default:
      RTC_LOG(LS_INFO)
          << "Ignore user agent information because already connected.";
  }
}
void P2PPeerConnectionChannel::OnMessageStop() {
  RTC_LOG(LS_INFO) << "Remote user stopped.";
  ClosePeerConnection();
  ChangeSessionState(kSessionStateReady);
}
void P2PPeerConnectionChannel::OnMessageSignal(Json::Value& message) {
  RTC_LOG(LS_INFO) << "OnMessageSignal";
  string type;
  string desc;
  rtc::GetStringFromJsonObject(message, kSessionDescriptionTypeKey, &type);
  RTC_LOG(LS_INFO) << "Received message type: " << type;
  // Store reference so peer_connection_ is not deleted until function ends
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> temp_pc_ = GetPeerConnectionRef();
  if (!temp_pc_) { return; }
  if (type == "offer" || type == "answer") {
    if (type == "offer" && session_state_ == kSessionStateMatched) {
      ChangeSessionState(kSessionStateConnecting);
    }
    string sdp;
    if (!rtc::GetStringFromJsonObject(message, kSessionDescriptionSdpKey,
                                      &sdp)) {
      RTC_LOG(LS_WARNING) << "Cannot parse received sdp.";
      return;
    }
    std::unique_ptr<webrtc::SessionDescriptionInterface> desc(
        webrtc::CreateSessionDescription(type, sdp, nullptr));
    if (!desc) {
      RTC_LOG(LS_ERROR) << "Failed to create session description.";
      return;
    }
    if (type == "offer" && temp_pc_ && temp_pc_->signaling_state() != webrtc::PeerConnectionInterface::kStable) {
      RTC_LOG(LS_INFO) << "Signaling state is " << temp_pc_->signaling_state()
                       << ", set SDP later.";
      pending_remote_sdp_ = std::move(desc);
    } else {
      scoped_refptr<FunctionalSetRemoteDescriptionObserver> observer =
          FunctionalSetRemoteDescriptionObserver::Create(std::bind(
              &P2PPeerConnectionChannel::OnSetRemoteDescriptionComplete, this,
              std::placeholders::_1));
      std::string sdp_string;
      if (!desc->ToString(&sdp_string)) {
        RTC_LOG(LS_ERROR) << "Error parsing local description.";
        RTC_DCHECK(false);
      }
      std::vector<AudioCodec> audio_codecs;
      for (auto& audio_enc_param : configuration_.audio) {
        audio_codecs.push_back(audio_enc_param.codec.name);
      }
      sdp_string = SdpUtils::SetPreferAudioCodecs(sdp_string, audio_codecs);
      std::vector<VideoCodec> video_codecs;
      for (auto& video_enc_param : configuration_.video) {
        video_codecs.push_back(video_enc_param.codec.name);
      }
      sdp_string = SdpUtils::SetPreferVideoCodecs(sdp_string, video_codecs);
      std::unique_ptr<webrtc::SessionDescriptionInterface> new_desc(
          webrtc::CreateSessionDescription(desc->type(), sdp_string, nullptr));
      // Synchronous call. After done, will invoke OnSetRemoteDescription. If
      // remote sent an offer, we create answer for it.
      RTC_LOG(LS_WARNING) << "SetRemoteSdp:" << sdp_string;
      temp_pc_->SetRemoteDescription(std::move(new_desc), observer);
      pending_remote_sdp_.reset();
    }
  } else if (type == "candidates") {
    string sdp_mid;
    string candidate;
    int sdp_mline_index;
    rtc::GetStringFromJsonObject(message, kIceCandidateSdpMidKey, &sdp_mid);
    rtc::GetStringFromJsonObject(message, kIceCandidateSdpNameKey, &candidate);
    rtc::GetIntFromJsonObject(message, kIceCandidateSdpMLineIndexKey,
                            &sdp_mline_index);
    webrtc::IceCandidateInterface* ice_candidate = webrtc::CreateIceCandidate(
        sdp_mid, sdp_mline_index, candidate, nullptr);
    temp_pc_->AddIceCandidate(ice_candidate);
  }
}
void P2PPeerConnectionChannel::OnMessageTracksAdded(
    Json::Value& stream_tracks) {
  // Find the streams with track information, and add them to published stream
  // list.
  for (Json::Value::ArrayIndex idx = 0; idx != stream_tracks.size(); idx++) {
    std::string track_id = stream_tracks[idx].asString();
    std::lock_guard<std::mutex> lock(local_stream_tracks_info_mutex_);
    if (local_stream_tracks_info_.find(track_id) !=
        local_stream_tracks_info_.end()) {
      std::string stream_label = local_stream_tracks_info_[track_id];
      {
        std::lock_guard<std::mutex> lock(published_streams_mutex_);
        auto it = published_streams_.find(stream_label);
        if (it == published_streams_.end())
          published_streams_.insert(stream_label);
        auto it_publishing = publishing_streams_.find(stream_label);
        if (it_publishing != publishing_streams_.end())
          publishing_streams_.erase(it_publishing);
      }
    }
  }
}

void P2PPeerConnectionChannel::OnMessageTrackSources(
    Json::Value& track_sources) {
  string id;
  string source;
  for (Json::Value::ArrayIndex idx = 0; idx != track_sources.size(); idx++) {
    rtc::GetStringFromJsonObject(track_sources[idx], kTrackIdKey, &id);
    rtc::GetStringFromJsonObject(track_sources[idx], kTrackSourceKey, &source);
    // Track source information collect.
    std::pair<std::string, std::string> track_source_info;
    track_source_info = std::make_pair(id, source);
    rtc::CritScope cs(&remote_track_source_info_crit_);
    remote_track_source_info_.insert(track_source_info);
  }
}
void P2PPeerConnectionChannel::OnMessageStreamInfo(Json::Value& stream_info) {
  // Stream information is useless in native layer.
}
void P2PPeerConnectionChannel::OnSignalingChange(
    PeerConnectionInterface::SignalingState new_state) {
  RTC_LOG(LS_INFO) << "Signaling state changed: " << new_state;
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> temp_pc_;
  switch (new_state) {
    case PeerConnectionInterface::SignalingState::kStable:
      if (pending_remote_sdp_) {
	RTC_LOG(LS_INFO) << "Retrying SetRemoteDescription from kStable state";
        scoped_refptr<FunctionalSetRemoteDescriptionObserver> observer =
            FunctionalSetRemoteDescriptionObserver::Create(std::bind(
                &P2PPeerConnectionChannel::OnSetRemoteDescriptionComplete, this,
                std::placeholders::_1));
        std::string sdp_string;
        if (!pending_remote_sdp_->ToString(&sdp_string)) {
          RTC_LOG(LS_ERROR) << "Error parsing local description.";
          RTC_DCHECK(false);
        }
        std::vector<AudioCodec> audio_codecs;
        for (auto& audio_enc_param : configuration_.audio) {
          audio_codecs.push_back(audio_enc_param.codec.name);
        }
        sdp_string = SdpUtils::SetPreferAudioCodecs(sdp_string, audio_codecs);
        std::vector<VideoCodec> video_codecs;
        for (auto& video_enc_param : configuration_.video) {
          video_codecs.push_back(video_enc_param.codec.name);
        }
        sdp_string = SdpUtils::SetPreferVideoCodecs(sdp_string, video_codecs);
        std::unique_ptr<webrtc::SessionDescriptionInterface> new_desc(
            webrtc::CreateSessionDescription(pending_remote_sdp_->type(),
                                             sdp_string, nullptr));
        pending_remote_sdp_.reset();
        peer_connection_->SetRemoteDescription(std::move(new_desc), observer);
      } else {
        DrainPendingStreams();
      }
      break;
    default:
      break;
  }
}
void P2PPeerConnectionChannel::OnAddStream(
    rtc::scoped_refptr<MediaStreamInterface> stream) {
  Json::Value stream_tracks;
  {
    rtc::CritScope cs(&remote_track_source_info_crit_);
    for (const auto& track : stream->GetAudioTracks()) {
      stream_tracks.append(track->id());
    }
    for (const auto& track : stream->GetVideoTracks()) {
      stream_tracks.append(track->id());
    }
  }
  std::shared_ptr<RemoteStream> remote_stream(
      new RemoteStream(stream, remote_id_));
  EventTrigger::OnEvent1<P2PPeerConnectionChannelObserver*,
                         std::allocator<P2PPeerConnectionChannelObserver*>,
                         void (owt::p2p::P2PPeerConnectionChannelObserver::*)(
                             std::shared_ptr<RemoteStream>),
                         std::shared_ptr<RemoteStream>>(
      observers_, event_queue_,
      &P2PPeerConnectionChannelObserver::OnStreamAdded, remote_stream);
  remote_streams_[stream->id()] = remote_stream;
  // Send the ack for the newly added stream tracks.
  Json::Value json_tracks;
  json_tracks[kMessageTypeKey] = kChatTracksAdded;
  json_tracks[kMessageDataKey] = stream_tracks;
  SendSignalingMessage(json_tracks);
}
void P2PPeerConnectionChannel::OnRemoveStream(
    rtc::scoped_refptr<MediaStreamInterface> stream) {
  if (remote_streams_.find(stream->id()) == remote_streams_.end()) {
    RTC_LOG(LS_WARNING) << "Remove an invalid stream.";
    RTC_DCHECK(false);
    return;
  }
  std::shared_ptr<RemoteStream> remote_stream = remote_streams_[stream->id()];
  remote_stream->TriggerOnStreamEnded();
  remote_streams_.erase(stream->id());
  rtc::CritScope cs(&remote_track_source_info_crit_);
  for (const auto& track : stream->GetAudioTracks())
    remote_track_source_info_.erase(track->id());
  for (const auto& track : stream->GetVideoTracks())
    remote_track_source_info_.erase(track->id());
}
void P2PPeerConnectionChannel::OnDataChannel(
    rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) {
  // If a new data channel is create, delete the old one to save resource.
  // Currently only one data channel for one connection. If we are going to
  // support multiple data channels(one for text, one for large files), replace
  // |data_channel_| with a map.
  if (data_channel_) {
    data_channel_->UnregisterObserver();
  }
  data_channel_ = data_channel;
  data_channel_->RegisterObserver(this);
}
void P2PPeerConnectionChannel::OnNegotiationNeeded() {
  RTC_LOG(LS_INFO) << "On negotiation needed.";
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> temp_pc_ = GetPeerConnectionRef();
  if (temp_pc_ && temp_pc_->signaling_state() == PeerConnectionInterface::SignalingState::kStable) {
    CreateOffer();
  } else if (temp_pc_) {
    negotiation_needed_ = true;
  }
}
void P2PPeerConnectionChannel::OnRenegotiationNeeded() {
  RTC_LOG(LS_ERROR) << "OnRenegotiationNeeded";
  CreateOffer();
}

void P2PPeerConnectionChannel::OnIceConnectionChange(
    PeerConnectionInterface::IceConnectionState new_state) {
  RTC_LOG(LS_INFO) << "Ice connection state changed: " << new_state;
  switch (new_state) {
    case webrtc::PeerConnectionInterface::kIceConnectionConnected:
    case webrtc::PeerConnectionInterface::kIceConnectionCompleted:
      ChangeSessionState(kSessionStateConnected);
      // reset |last_disconnect_|.
      {
        std::lock_guard<std::mutex> lock(last_disconnect_mutex_);
        last_disconnect_ =
            std::chrono::time_point<std::chrono::system_clock>::max();
      }
      DrainPendingStreams();
      break;
    case webrtc::PeerConnectionInterface::kIceConnectionDisconnected:
      {
        std::lock_guard<std::mutex> lock(last_disconnect_mutex_);
        last_disconnect_ = std::chrono::system_clock::now();
      }
      // Check state after a period of time.
      std::thread([this]() {
        // Extends lifetime of this with ref until detached thread exits.
	auto ref = shared_from_this();
        std::this_thread::sleep_for(std::chrono::seconds(reconnect_timeout_));
	int secs_since_disconnect = 0;
	{
	  std::lock_guard<std::mutex> lock(last_disconnect_mutex_);
          if (last_disconnect_ != std::chrono::time_point<std::chrono::system_clock>::max()) {
	    secs_since_disconnect = std::chrono::duration_cast<std::chrono::seconds>(
		std::chrono::system_clock::now() - last_disconnect_).count();
	  }
	}
	if (secs_since_disconnect >= reconnect_timeout_) {
          RTC_LOG(LS_INFO) << "Detect reconnection failed, stop this session.";
          Stop(nullptr, nullptr);
        } else {
          RTC_LOG(LS_INFO) << "Detect reconnection succeed.";
        }
      }).detach();
      break;
    case webrtc::PeerConnectionInterface::kIceConnectionClosed:
      CleanLastPeerConnection();
      break;
    case webrtc::PeerConnectionInterface::kIceConnectionFailed:
      for (std::unordered_map<std::string,
                              std::shared_ptr<RemoteStream>>::iterator it =
               remote_streams_.begin();
           it != remote_streams_.end(); it++)
        it->second->TriggerOnStreamEnded();
      remote_streams_.clear();
      {
        rtc::CritScope cs(&remote_track_source_info_crit_);
        remote_track_source_info_.clear();
      }
      break;
    default:
      break;
  }
}
void P2PPeerConnectionChannel::OnIceGatheringChange(
    PeerConnectionInterface::IceGatheringState new_state) {
  RTC_LOG(LS_INFO) << "Ice gathering state changed: " << new_state;
}
void P2PPeerConnectionChannel::OnIceCandidate(
    const webrtc::IceCandidateInterface* candidate) {
  RTC_LOG(LS_INFO) << "On ice candidate";
  Json::Value signal;
  signal[kSessionDescriptionTypeKey] = "candidates";
  signal[kIceCandidateSdpMLineIndexKey] = candidate->sdp_mline_index();
  signal[kIceCandidateSdpMidKey] = candidate->sdp_mid();
  string sdp;
  if (!candidate->ToString(&sdp)) {
    RTC_LOG(LS_ERROR) << "Failed to serialize candidate";
    return;
  }
  signal[kIceCandidateSdpNameKey] = sdp;
  Json::Value json;
  json[kMessageTypeKey] = kChatSignal;
  json[kMessageDataKey] = signal;
  SendSignalingMessage(json);
}
void P2PPeerConnectionChannel::OnCreateSessionDescriptionSuccess(
    webrtc::SessionDescriptionInterface* desc) {
  RTC_LOG(LS_INFO) << "Create sdp success.";
  {
    std::lock_guard<std::mutex> lock(ended_mutex_);
    if (ended_) { return; }
  }
  scoped_refptr<FunctionalSetSessionDescriptionObserver> observer =
      FunctionalSetSessionDescriptionObserver::Create(
          std::bind(
              &P2PPeerConnectionChannel::OnSetLocalSessionDescriptionSuccess,
              this),
          std::bind(
              &P2PPeerConnectionChannel::OnSetLocalSessionDescriptionFailure,
              this, std::placeholders::_1));
  peer_connection_->SetLocalDescription(observer);
}
void P2PPeerConnectionChannel::OnCreateSessionDescriptionFailure(
    const std::string& error) {
  RTC_LOG(LS_ERROR) << "Create sdp failed. " << error;
  Stop(nullptr, nullptr);
}
void P2PPeerConnectionChannel::OnSetLocalSessionDescriptionSuccess() {
  RTC_LOG(LS_INFO) << "Set local sdp success.";
  {
    std::lock_guard<std::mutex> lock(is_creating_offer_mutex_);
    is_creating_offer_ = false;
  }
  {
    std::lock_guard<std::mutex> lock(ended_mutex_);
    if (ended_) { return; }
  }
  // Setting maximum bandwidth here.
  ApplyBitrateSettings();
  auto desc = LocalDescription();
  string sdp;
  desc->ToString(&sdp);
  Json::Value signal;
  signal[kSessionDescriptionTypeKey] = desc->type();
  signal[kSessionDescriptionSdpKey] = sdp;
  Json::Value json;
  json[kMessageTypeKey] = kChatSignal;
  json[kMessageDataKey] = signal;
  // The fourth signaling message of SDP to remote peer.
  SendSignalingMessage(json);
}
void P2PPeerConnectionChannel::OnSetLocalSessionDescriptionFailure(
    const std::string& error) {
  RTC_LOG(LS_ERROR) << "Set local sdp failed. " << error;
  Stop(nullptr, nullptr);
}
void P2PPeerConnectionChannel::OnSetRemoteSessionDescriptionSuccess() {
  {
    std::lock_guard<std::mutex> lock(ended_mutex_);
    if (ended_) { return; }
  }
  PeerConnectionChannel::OnSetRemoteSessionDescriptionSuccess();
}
void P2PPeerConnectionChannel::OnSetRemoteSessionDescriptionFailure(
    const std::string& error) {
  RTC_LOG(LS_ERROR) << "Set remote sdp failed. " << error;
  Stop(nullptr, nullptr);
}
bool P2PPeerConnectionChannel::CheckNullPointer(
    uintptr_t pointer,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  if (pointer)
    return true;
  if (on_failure != nullptr) {
    std::unique_ptr<Exception> e(new Exception(
        ExceptionType::kP2PClientInvalidArgument, "Nullptr is not allowed."));
    on_failure(std::move(e));
  }
  return false;
}

void P2PPeerConnectionChannel::TriggerOnStopped() {
  for (std::vector<P2PPeerConnectionChannelObserver*>::iterator it =
       observers_.begin(); it != observers_.end(); it++) {
    (*it)->OnStopped(remote_id_);
  }
}

void P2PPeerConnectionChannel::CleanLastPeerConnection() {
  pending_remote_sdp_.reset();
  negotiation_needed_ = false;
  {
    std::lock_guard<std::mutex> lock(last_disconnect_mutex_);
    last_disconnect_ = std::chrono::time_point<std::chrono::system_clock>::max();
  }
}
void P2PPeerConnectionChannel::Unpublish(
    std::shared_ptr<LocalStream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> temp_pc_ = GetPeerConnectionRef();
  if (!CheckNullPointer((uintptr_t)stream.get(), on_failure)) {
    RTC_LOG(LS_WARNING) << "Local stream cannot be nullptr.";
    return;
  }
  if (!remote_side_supports_remove_stream_) {
    if (on_failure != nullptr) {
      RTC_LOG(LS_WARNING) << "Remote side does not support removeStream.";
      std::unique_ptr<Exception> e(
          new Exception(ExceptionType::kP2PClientUnsupportedMethod,
                        "Remote side does not support unpublish."));
      on_failure(std::move(e));
    }
    return;
  }
  RTC_CHECK(stream->MediaStream());
  // Calling MediaStream->id() runs on signaling_thread, so must be outside locks.
  std::string stream_id = stream->MediaStream()->id();
  {
    std::lock_guard<std::mutex> lock(published_streams_mutex_);
    auto it = published_streams_.find(stream_id);
    if (it == published_streams_.end()) {
      if (on_failure) {
        std::unique_ptr<Exception> e(
            new Exception(ExceptionType::kP2PClientInvalidArgument,
                          "The stream is not published."));
        on_failure(std::move(e));
      }
      return;
    }
    published_streams_.erase(it);
  }
  {
    std::lock_guard<std::mutex> lock(pending_unpublish_streams_mutex_);
    pending_unpublish_streams_.push_back(stream);
  }
  if (on_success) {
    on_success();
  }
  DrainPendingStreams();
}
void P2PPeerConnectionChannel::Stop(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  RTC_LOG(LS_INFO) << "Stop session.";
  switch (session_state_) {
    case kSessionStateConnecting:
    case kSessionStateConnected:
    case kSessionStateMatched:
    case kSessionStateOffered:

      SendStop(nullptr, nullptr);
      ClosePeerConnection();
      {
        std::lock_guard<std::mutex> lock(stop_send_mutex_);
        stop_send_needed_ = false;
      }
      ChangeSessionState(kSessionStateReady);
      break;
    default:
      if (on_failure != nullptr) {
        std::unique_ptr<Exception> e(
            new Exception(ExceptionType::kP2PClientInvalidState,
                          "Cannot stop a session haven't started."));
        on_failure(std::move(e));
      }
      return;
  }
  if (on_success != nullptr) {
    on_success();
  }
}
void P2PPeerConnectionChannel::GetConnectionStats(
    std::function<void(std::shared_ptr<ConnectionStats>)> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> temp_pc_ = GetPeerConnectionRef();
  if (on_success == nullptr) {
    if (on_failure != nullptr) {
      std::unique_ptr<Exception> e(
          new Exception(ExceptionType::kP2PClientInvalidArgument,
                        "on_success cannot be nullptr. Please provide "
                        "on_success to get connection stats data."));
      on_failure(std::move(e));
    }
    return;
  }
  rtc::scoped_refptr<FunctionalStatsObserver> observer =
      FunctionalStatsObserver::Create(std::move(on_success));
  if (temp_pc_) {
    temp_pc_->GetStats(
      observer, nullptr,
      webrtc::PeerConnectionInterface::kStatsOutputLevelStandard);
  }
}

void P2PPeerConnectionChannel::GetConnectionStats(
    std::function<void(std::shared_ptr<RTCStatsReport>)> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> temp_pc_ = GetPeerConnectionRef();
  if (on_success == nullptr) {
    if (on_failure != nullptr) {
      std::unique_ptr<Exception> e(
          new Exception(ExceptionType::kP2PClientInvalidArgument,
                        "on_success cannot be nullptr. Please provide "
                        "on_success to get connection stats data."));
      on_failure(std::move(e));
    }
    return;
  }
  rtc::scoped_refptr<FunctionalStandardRTCStatsCollectorCallback> observer =
      FunctionalStandardRTCStatsCollectorCallback::Create(
          std::move(on_success));
  if (temp_pc_) {
    temp_pc_->GetStats(observer);
  }
}

void P2PPeerConnectionChannel::GetStats(
    std::function<void(const webrtc::StatsReports& reports)> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> temp_pc_ = GetPeerConnectionRef();
  if (on_success == nullptr) {
    if (on_failure != nullptr) {
      std::unique_ptr<Exception> e(
          new Exception(ExceptionType::kP2PClientInvalidArgument,
                        "on_success cannot be nullptr. Please provide "
                        "on_success to get connection stats data."));
      on_failure(std::move(e));
    }
    return;
  }
  if (session_state_ != kSessionStateConnected) {
    if (on_failure != nullptr) {
      std::unique_ptr<Exception> e(
          new Exception(ExceptionType::kP2PClientInvalidState,
                        "Cannot get connection stats in this state. Please "
                        "try it after connection is established."));
      on_failure(std::move(e));
    }
    return;
  }
  RTC_LOG(LS_INFO) << "Get native stats";
  rtc::scoped_refptr<FunctionalNativeStatsObserver> observer =
      FunctionalNativeStatsObserver::Create(std::move(on_success));
  if (temp_pc_) {
    temp_pc_->GetStats(
        observer, nullptr,
        webrtc::PeerConnectionInterface::kStatsOutputLevelStandard);
  }
}
bool P2PPeerConnectionChannel::HaveLocalOffer() {
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> temp_pc_ = GetPeerConnectionRef();
  if (temp_pc_) {
    return temp_pc_->signaling_state() == webrtc::PeerConnectionInterface::kHaveLocalOffer;
  } else {
    return false;
  }
}
std::shared_ptr<LocalStream> P2PPeerConnectionChannel::GetLatestLocalStream() {
  return latest_local_stream_;
}
std::function<void()>
P2PPeerConnectionChannel::GetLatestPublishSuccessCallback() {
  return latest_publish_success_callback_;
}
std::function<void(std::unique_ptr<Exception>)>
P2PPeerConnectionChannel::GetLatestPublishFailureCallback() {
  return latest_publish_failure_callback_;
}
bool P2PPeerConnectionChannel::IsAbandoned() {
  return remote_side_offline_;
}
bool P2PPeerConnectionChannel::IsStale() {
  bool is_stale = false;
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> temp_pc_ = GetPeerConnectionRef();
  if (temp_pc_) {
    auto now = std::chrono::system_clock::now();
    auto age = std::chrono::duration_cast<std::chrono::seconds>(now - created_time_).count();
    auto ice_state = temp_pc_->ice_connection_state();
    auto signaling_state = temp_pc_->signaling_state();
    is_stale = (age > kStaleThresholdSecs) &&
               ((ice_state == webrtc::PeerConnectionInterface::kIceConnectionNew)
                || (ice_state == webrtc::PeerConnectionInterface::kIceConnectionFailed) &&
                   (signaling_state == webrtc::PeerConnectionInterface::kHaveLocalOffer));
    std::cout << "(" << age <<  " > " << kStaleThresholdSecs << ") && (("
              << ice_state << " == " << webrtc::PeerConnectionInterface::kIceConnectionNew << ") || ("
              << ice_state << " == " << webrtc::PeerConnectionInterface::kIceConnectionFailed << ") && ("
              << signaling_state << " == " << webrtc::PeerConnectionInterface::kHaveLocalOffer << ")) = "
              << is_stale
              << std::endl;
  } else {
    is_stale = true;
  }
  return is_stale;
}
void P2PPeerConnectionChannel::DrainPendingStreams() {
  RTC_LOG(LS_INFO) << "Draining pending stream";
  ChangeSessionState(kSessionStateConnecting);
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> temp_pc_ = GetPeerConnectionRef();
  // Move contents of pending vectors to a temporary vector so
  // lock can be released before iterating over the streams.
  // First to publish the pending_publish_streams_ list.
  std::vector<std::shared_ptr<LocalStream>> publish_snapshot;
  {
    std::lock_guard<std::mutex> lock(pending_publish_streams_mutex_);
    for (auto it = pending_publish_streams_.begin();
         it != pending_publish_streams_.end(); ++it) {
      std::shared_ptr<LocalStream> stream = *it;
      publish_snapshot.push_back(stream);
    }
    pending_publish_streams_.clear();
  }
  // Then to unpublish the pending_unpublish_streams_ list.
  std::vector<std::shared_ptr<LocalStream>> unpublish_snapshot;
  {
    std::lock_guard<std::mutex> lock(pending_unpublish_streams_mutex_);
    for (auto it = pending_unpublish_streams_.begin();
         it != pending_unpublish_streams_.end(); ++it) {
      std::shared_ptr<LocalStream> stream = *it;
      unpublish_snapshot.push_back(stream);
    }
    pending_unpublish_streams_.clear();
  }
  // Snapshot vectors have no contention, safe to call webrtc methods.
  if (temp_pc_) {
    // Snapshot vector has no thread contention, safe to call webrtc methods.
    for (auto it = publish_snapshot.begin(); it != publish_snapshot.end(); ++it) {
      std::shared_ptr<LocalStream> stream = *it;
      std::string audio_track_source = "mic";
      std::string video_track_source = "camera";
      if (stream->Source().audio == owt::base::AudioSourceInfo::kScreenCast) {
        audio_track_source = "screen-cast";
      }
      if (stream->Source().video == owt::base::VideoSourceInfo::kScreenCast) {
        video_track_source = "screen-cast";
      }
      RTC_CHECK(temp_pc_);
      // Collect stream and tracks information.
      rtc::scoped_refptr<webrtc::MediaStreamInterface> media_stream =
          stream->MediaStream();
      std::string stream_id = stream->Id();
      Json::Value track_info;
      Json::Value track_sources;
      Json::Value stream_tracks;
      Json::Value stream_sources;
      for (const auto& track : media_stream->GetAudioTracks()) {
        // Signaling.
        stream_tracks.append(track->id());
        stream_sources[kStreamAudioSourceKey] = audio_track_source;
        track_info[kTrackIdKey] = track->id();
        track_info[kTrackSourceKey] = audio_track_source;
        track_sources.append(track_info);
        temp_pc_->AddTrack(track, {stream_id});
      }
      for (const auto& track : media_stream->GetVideoTracks()) {
        // Signaling.
        stream_tracks.append(track->id());
        stream_sources[kStreamVideoSourceKey] = video_track_source;
        track_info[kTrackIdKey] = track->id();
        track_info[kTrackSourceKey] = video_track_source;
        track_sources.append(track_info);
        temp_pc_->AddTrack(track, {stream_id});
      }
      // The second signaling message of track sources to remote peer.
      Json::Value json_track_sources;
      json_track_sources[kMessageTypeKey] = kChatTrackSources;
      json_track_sources[kMessageDataKey] = track_sources;
      SendSignalingMessage(json_track_sources);
      // The third signaling message of stream information to remote peer.
      Json::Value json_stream_info;
      json_stream_info[kMessageTypeKey] = kChatStreamInfo;
      Json::Value stream_info;
      stream_info[kStreamIdKey] = media_stream->id();
      stream_info[kStreamTracksKey] = stream_tracks;
      stream_info[kStreamSourceKey] = stream_sources;
      json_stream_info[kMessageDataKey] = stream_info;
      SendSignalingMessage(json_stream_info);
    }
    for (auto it = unpublish_snapshot.begin(); it != unpublish_snapshot.end(); ++it) {
      std::shared_ptr<LocalStream> stream = *it;
      scoped_refptr<webrtc::MediaStreamInterface> media_stream =
          stream->MediaStream();
      RTC_CHECK(temp_pc_);
      for (const auto& track : media_stream->GetAudioTracks()) {
        const auto& senders = temp_pc_->GetSenders();
        for (auto& s : senders) {
          const auto& t = s->track();
          if (t != nullptr && t->id() == track->id()) {
            temp_pc_->RemoveTrack(s);
            break;
          }
        }

      }
      for (const auto& track : media_stream->GetVideoTracks()) {
        const auto& senders = temp_pc_->GetSenders();
        for (auto& s : senders) {
          const auto& t = s->track();
          if (t != nullptr && t->id() == track->id()) {
            temp_pc_->RemoveTrack(s);
            break;
          }
        }
      }
    }
  }
  publish_snapshot.clear();
  unpublish_snapshot.clear();
}
void P2PPeerConnectionChannel::SendStop(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  RTC_LOG(LS_INFO) << "Send stop.";
  Json::Value json;
  json[kMessageTypeKey] = kChatClosed;
  SendSignalingMessage(json, on_success, on_failure);
}
// Only function that holds locks while calling WebRTC methods. Any deadlock is here.
void P2PPeerConnectionChannel::ClosePeerConnection() {
  RTC_LOG(LS_INFO) << "Close peer connection.";
  // Reference to peer connection  that outlives the scope of the locks.
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> temp_pc_;
  rtc::scoped_refptr<webrtc::DataChannelInterface> temp_dc_;
  {
    std::lock_guard<std::mutex> lock(ended_mutex_);
    if (!ended_) {
      ended_ = true;
      temp_pc_ = peer_connection_;
      temp_dc_ = data_channel_;
      peer_connection_ = nullptr;
      data_channel_ = nullptr;
    }
  }
  if (temp_pc_) {
    temp_pc_->Close();
    {
      std::lock_guard<std::mutex> lock(pending_unpublish_streams_mutex_);
      pending_unpublish_streams_.clear();
    }
    {
      std::lock_guard<std::mutex> lock(pending_publish_streams_mutex_);
      pending_publish_streams_.clear();
    }
    TriggerOnStopped();
  }
}
void P2PPeerConnectionChannel::CheckWaitedList() {
  RTC_LOG(LS_INFO) << "CheckWaitedList";
  if (!pending_publish_streams_.empty() ||
      !pending_unpublish_streams_.empty()) {
    DrainPendingStreams();
  } else if (negotiation_needed_) {
    CreateOffer();
  }
}

void P2PPeerConnectionChannel::OnDataChannelStateChange() {
  if (data_channel_ && data_channel_->state() ==
      webrtc::DataChannelInterface::DataState::kOpen) {
    DrainPendingMessages();
  }
}
void P2PPeerConnectionChannel::OnDataChannelMessage(
    const webrtc::DataBuffer& buffer) {
  if (buffer.binary) {
    RTC_LOG(LS_WARNING) << "Binary data is not supported.";
    return;
  }
  std::string data = std::string(buffer.data.data<char>(), buffer.data.size());
  // Parse the received message with its id and data.
  Json::Reader reader;
  Json::Value json_data;
  if (!reader.parse(data, json_data)) {
    RTC_LOG(LS_WARNING) << "Cannot parse incoming text message.";
    return;
  }
  std::string message_id;
  rtc::GetStringFromJsonObject(json_data, kTextMessageIdKey, &message_id);
  if (message_id.empty()) {
    RTC_LOG(LS_WARNING) << "Cannot get id from incoming text message.";
    return;
  }
  std::string message;
  rtc::GetStringFromJsonObject(json_data, kTextMessageDataKey, &message);
  if (message.empty()) {
    RTC_LOG(LS_WARNING) << "Cannot get content from incoming text message.";
    return;
  }
  // Send the ack for text message.
  if (!remote_side_ignores_datachannel_acks_) {
    Json::Value ack;
    ack[kMessageTypeKey] = kChatDataReceived;
    ack[kMessageDataKey] = message_id;
    SendSignalingMessage(ack);
  }
  // Deal with the received text message.
  for (std::vector<P2PPeerConnectionChannelObserver*>::iterator it =
           observers_.begin();
       it != observers_.end(); ++it) {
    (*it)->OnMessageReceived(remote_id_, message);
  }
}
void P2PPeerConnectionChannel::CreateDataChannel(const std::string& label) {
  webrtc::DataChannelInit config;
  data_channel_ = peer_connection_->CreateDataChannel(label, &config);
  if (data_channel_)
    data_channel_->RegisterObserver(this);
}
webrtc::DataBuffer P2PPeerConnectionChannel::CreateDataBuffer(
    const std::string& data) {
  rtc::CopyOnWriteBuffer buffer(data.c_str(), data.length());
  webrtc::DataBuffer data_buffer(buffer, false);
  return data_buffer;
}
void P2PPeerConnectionChannel::DrainPendingMessages() {
  std::vector<std::string> messages_snapshot;
  rtc::scoped_refptr<webrtc::DataChannelInterface> temp_dc_;
  {
    std::lock_guard<std::mutex> lock(ended_mutex_);
    if (!ended_) {
      temp_dc_ = data_channel_;
    }
  }
  if (temp_dc_) {
    std::lock_guard<std::mutex> lock(pending_messages_mutex_);
    RTC_LOG(LS_INFO) << "Draining pending messages. Message queue size: "
                   << pending_messages_.size();
    for (const auto& m : pending_messages_) {
      messages_snapshot.push_back(m);
    }
    pending_messages_.clear();

    // After releasing locks, safe to call Send
    for (const auto& msg : messages_snapshot) {
      temp_dc_->Send(CreateDataBuffer(msg));
    }
  }
}
Json::Value P2PPeerConnectionChannel::UaInfo() {
  Json::Value ua;
  // SDK info includes verison and type.
  Json::Value sdk;
  SysInfo sys_info = SysInfo::GetInstance();
  sdk[kUaSdkVersionKey] = sys_info.sdk.version;
  sdk[kUaSdkTypeKey] = sys_info.sdk.type;
  // Runtime values  with system information.
  Json::Value runtime;
  runtime[kUaRuntimeNameKey] = sys_info.runtime.name;
  runtime[kUaRuntimeVersionKey] = sys_info.runtime.version;
  // OS info includes OS name and OS version.
  Json::Value os;
  os[kUaOsNameKey] = sys_info.os.name;
  os[kUaOsVersionKey] = sys_info.os.version;
  // Capabilities and customized configuration
  // TODO: currently default to support continual ICE gathering,
  // Plan-B, and stream removable.
  Json::Value capabilities;
  capabilities[kUaContinualGatheringKey] = true;
  capabilities[kUaUnifiedPlanKey] = true;
  capabilities[kUaStreamRemovableKey] = true;
  capabilities[kUaIgnoresDataChannelAcksKey] = true;
  ua[kUaSdkKey] = sdk;
  ua[kUaRuntimeKey] = runtime;
  ua[kUaOsKey] = os;
  ua[kUaCapabilitiesKey] = capabilities;
  return ua;
}
void P2PPeerConnectionChannel::HandleRemoteCapability(Json::Value& ua) {
  Json::Value capabilities;
  rtc::GetValueFromJsonObject(ua, kUaCapabilitiesKey, &capabilities);
  rtc::GetBoolFromJsonObject(capabilities, kUaContinualGatheringKey,
                             &remote_side_supports_continual_ice_gathering_);
  rtc::GetBoolFromJsonObject(capabilities, kUaUnifiedPlanKey,
                             &remote_side_supports_unified_plan_);
  remote_side_supports_plan_b_ = !remote_side_supports_unified_plan_;
  // Firefox is now on unified plan semantics, so supports removing streams.
  rtc::GetBoolFromJsonObject(capabilities, kUaIgnoresDataChannelAcksKey,
                             &remote_side_ignores_datachannel_acks_);
  remote_side_supports_remove_stream_ = true;
  RTC_LOG(LS_INFO) << "Remote side supports removing stream? "
                   << remote_side_supports_remove_stream_;
  RTC_LOG(LS_INFO) << "Remote side supports WebRTC Plan B? "
                   << remote_side_supports_plan_b_;
  RTC_LOG(LS_INFO) << "Remote side supports WebRTC Unified Plan?"
                   << remote_side_supports_unified_plan_;
  RTC_LOG(LS_INFO) << "Remote side ignores data channel acks?"
                   << remote_side_ignores_datachannel_acks_;
}
void P2PPeerConnectionChannel::SendUaInfo() {
  Json::Value json;
  json[kMessageTypeKey] = kChatUserAgent;
  Json::Value ua = UaInfo();
  json[kMessageDataKey] = ua;
  SendSignalingMessage(json);
}
}  // namespace p2p
}  // namespace owt
