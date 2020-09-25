// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include <thread>
#include <vector>
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
      ended_(false) {
  RTC_CHECK(signaling_sender_);
  InitializePeerConnection();
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
  if (signaling_sender_)
    delete signaling_sender_;
  if (peer_connection_ != nullptr) {
    peer_connection_->Close();
  }
}
void P2PPeerConnectionChannel::Publish(
    std::shared_ptr<LocalStream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  RTC_LOG(LS_INFO) << "Publishing a local stream.";
  if (!CheckNullPointer((uintptr_t)stream.get(), on_failure)) {
    RTC_LOG(LS_INFO) << "Local stream cannot be nullptr.";
    return;
  }
  RTC_CHECK(stream->MediaStream());
  std::string stream_label = stream->MediaStream()->id();
  if (published_streams_.find(stream_label) != published_streams_.end() ||
      publishing_streams_.find(stream_label) != publishing_streams_.end()) {
    if (on_failure) {
      event_queue_->PostTask([on_failure] {
        std::unique_ptr<Exception> e(
            new Exception(ExceptionType::kP2PClientInvalidArgument,
                          "The stream is already published."));
        on_failure(std::move(e));
      });
    }
    return;
  }
  latest_local_stream_ = stream;
  latest_publish_success_callback_ = on_success;
  latest_publish_failure_callback_ = on_failure;
  // Send chat-closed to workaround known browser bugs, together with
  // user agent information once and once only.
  if (stop_send_needed_) {
    SendStop(nullptr, nullptr);
    stop_send_needed_ = false;
  }
  if (!ua_sent_) {
    SendUaInfo();
    ua_sent_ = true;
  }
  scoped_refptr<webrtc::MediaStreamInterface> media_stream =
      stream->MediaStream();
  std::pair<std::string, std::string> stream_track_info;
  for (const auto& track : media_stream->GetAudioTracks()) {
    std::lock_guard<std::mutex> lock(local_stream_tracks_info_mutex_);
    if (local_stream_tracks_info_.find(track->id()) ==
        local_stream_tracks_info_.end()) {
      stream_track_info = std::make_pair(track->id(), media_stream->id());
      local_stream_tracks_info_.insert(stream_track_info);
    }
  }
  for (const auto& track : media_stream->GetVideoTracks()) {
    std::lock_guard<std::mutex> lock(local_stream_tracks_info_mutex_);
    if (local_stream_tracks_info_.find(track->id()) ==
        local_stream_tracks_info_.end()) {
      stream_track_info = std::make_pair(track->id(), media_stream->id());
      local_stream_tracks_info_.insert(stream_track_info);
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
    // TODO: Here to directly call on_success on publication
    // publish_success_callbacks_[stream_label] = on_success;
    event_queue_->PostTask([on_success] { on_success(); });
  }
  if (on_failure) {
    std::lock_guard<std::mutex> lock(failure_callbacks_mutex_);
    failure_callbacks_[stream_label] = on_failure;
  }
  if (SignalingState() == PeerConnectionInterface::SignalingState::kStable)
    DrainPendingStreams();
}
void P2PPeerConnectionChannel::Send(
    const std::string& message,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  // Send chat-closed to workaround known browser bugs, together with
  // user agent information once and once only.
  if (stop_send_needed_) {
    SendStop(nullptr, nullptr);
    stop_send_needed_ = false;
  }
  if (!ua_sent_) {
    SendUaInfo();
    ua_sent_ = true;
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
  if (data_channel_ != nullptr &&
      data_channel_->state() == webrtc::DataChannelInterface::DataState::kOpen) {
    data_channel_->Send(CreateDataBuffer(data));
    if (on_success) {
      event_queue_->PostTask([on_success] { on_success(); });
    }
  } else {
    {
      std::lock_guard<std::mutex> lock(pending_messages_mutex_);
      std::shared_ptr<std::string> data_copy(
          std::make_shared<std::string>(data));
      pending_messages_.push_back(std::tuple<std::shared_ptr<std::string>,
              std::function<void()>,
              std::function<void(std::unique_ptr<Exception>)>>{data_copy, on_success, on_failure});
    }
    if (data_channel_ == nullptr) // Otherwise, wait for data channel ready.
      CreateDataChannel(kDataChannelLabelForTextMessage);
  }
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
  peer_connection_->CreateOffer(
      observer, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
}
void P2PPeerConnectionChannel::CreateAnswer() {
  RTC_LOG(LS_INFO) << "Create answer.";
  scoped_refptr<FunctionalCreateSessionDescriptionObserver> observer =
      FunctionalCreateSessionDescriptionObserver::Create(
          std::bind(
              &P2PPeerConnectionChannel::OnCreateSessionDescriptionSuccess,
              this, std::placeholders::_1),
          std::bind(
              &P2PPeerConnectionChannel::OnCreateSessionDescriptionFailure,
              this, std::placeholders::_1));
  peer_connection_->CreateAnswer(
      observer, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
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
          std::lock_guard<std::mutex> lock(failure_callbacks_mutex_);
          for (std::unordered_map<
                   std::string,
                   std::function<void(std::unique_ptr<Exception>)>>::iterator
                   it = failure_callbacks_.begin();
               it != failure_callbacks_.end(); it++) {
            std::function<void(std::unique_ptr<Exception>)> failure_callback =
                it->second;
            event_queue_->PostTask([failure_callback, type, msg] {
              std::unique_ptr<Exception> e(new Exception(type, msg));
              failure_callback(std::move(e));
            });
          }
          failure_callbacks_.clear();
        }
        if (on_failure)
          on_failure(std::move(exception));
      });
}
void P2PPeerConnectionChannel::OnIncomingSignalingMessage(
    const std::string& message) {
  if (ended_)
    return;
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
    // Send back user agent info once and only once.
    if (!ua_sent_) {
      SendUaInfo();
      ua_sent_ = true;
      stop_send_needed_ = false;
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
  {
    {
      std::lock_guard<std::mutex> lock(failure_callbacks_mutex_);
      for (std::unordered_map<
               std::string,
               std::function<void(std::unique_ptr<Exception>)>>::iterator it =
               failure_callbacks_.begin();
           it != failure_callbacks_.end(); it++) {
        std::function<void(std::unique_ptr<Exception>)> failure_callback =
            it->second;
        event_queue_->PostTask([failure_callback] {
          std::unique_ptr<Exception> e(
              new Exception(ExceptionType::kP2PClientInvalidArgument,
                            "Stop message received."));
          failure_callback(std::move(e));
        });
      }
      failure_callbacks_.clear();
    }
  }
  {
    std::lock_guard<std::mutex> lock(pending_messages_mutex_);
    for (auto it = pending_messages_.begin(); it != pending_messages_.end();
         ++it) {
      std::function<void(std::unique_ptr<Exception>)> on_failure;
      std::tie(std::ignore, std::ignore, on_failure) = *it;
      if (on_failure) {
        event_queue_->PostTask([on_failure] {
          std::unique_ptr<Exception> e(
              new Exception(ExceptionType::kP2PClientInvalidArgument,
                            "Stop message received."));
          on_failure(std::move(e));
        });
      }
    }
    pending_messages_.clear();
  }
  ClosePeerConnection();
  ChangeSessionState(kSessionStateReady);
}
void P2PPeerConnectionChannel::OnMessageSignal(Json::Value& message) {
  RTC_LOG(LS_INFO) << "OnMessageSignal";
  string type;
  string desc;
  rtc::GetStringFromJsonObject(message, kSessionDescriptionTypeKey, &type);
  RTC_LOG(LS_INFO) << "Received message type: " << type;
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
    if (type == "offer" &&
        SignalingState() != webrtc::PeerConnectionInterface::kStable) {
      RTC_LOG(LS_INFO) << "Signaling state is " << SignalingState()
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
      // Sychronous call. After done, will invoke OnSetRemoteDescription. If
      // remote sent an offer, we create answer for it.
      RTC_LOG(LS_WARNING) << "SetRemoteSdp:" << sdp_string;
      peer_connection_->SetRemoteDescription(std::move(new_desc), observer);
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
    peer_connection_->AddIceCandidate(ice_candidate);
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
      // Trigger the successful callback of publish.
      if (publish_success_callbacks_.find(stream_label) ==
          publish_success_callbacks_.end()) {
        RTC_LOG(LS_WARNING)
            << "No callback available for publishing stream with track id: "
            << track_id;
        return;
      }
      std::function<void()> callback = publish_success_callbacks_[stream_label];
      event_queue_->PostTask([callback] { callback(); });
      publish_success_callbacks_.erase(stream_label);
      // Remove the failure callback accordingly
      std::lock_guard<std::mutex> lock(failure_callbacks_mutex_);
      if (failure_callbacks_.find(stream_label) != failure_callbacks_.end())
        failure_callbacks_.erase(stream_label);
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
  switch (new_state) {
    case PeerConnectionInterface::SignalingState::kStable:
      if (pending_remote_sdp_) {
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
        event_queue_->PostTask([this] { DrainPendingStreams(); });
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
  if (data_channel_)
    data_channel_ = nullptr;
  data_channel_ = data_channel;
  data_channel_->RegisterObserver(this);
  DrainPendingMessages();
}
void P2PPeerConnectionChannel::OnNegotiationNeeded() {
  if (ended_)
    return;
  RTC_LOG(LS_INFO) << "On negotiation needed.";
  if (SignalingState() == PeerConnectionInterface::SignalingState::kStable) {
    CreateOffer();
  } else {
    negotiation_needed_ = true;
  }
}
void P2PPeerConnectionChannel::OnIceConnectionChange(
    PeerConnectionInterface::IceConnectionState new_state) {
  RTC_LOG(LS_INFO) << "Ice connection state changed: " << new_state;
  switch (new_state) {
    case webrtc::PeerConnectionInterface::kIceConnectionConnected:
    case webrtc::PeerConnectionInterface::kIceConnectionCompleted:
      ChangeSessionState(kSessionStateConnected);
      //CheckWaitedList();
      // reset |last_disconnect_|.
      last_disconnect_ =
          std::chrono::time_point<std::chrono::system_clock>::max();
      break;
    case webrtc::PeerConnectionInterface::kIceConnectionDisconnected:
      last_disconnect_ = std::chrono::system_clock::now();
      // Check state after a period of time.
      std::thread([this]() {
        std::this_thread::sleep_for(std::chrono::seconds(reconnect_timeout_));
        if (std::chrono::system_clock::now() - last_disconnect_ >=
            std::chrono::seconds(reconnect_timeout_)) {
          RTC_LOG(LS_INFO) << "Detect reconnection failed, stop this session.";
          Stop(nullptr, nullptr);
        } else {
          RTC_LOG(LS_INFO) << "Detect reconnection succeed.";
        }
      }).detach();
      break;
    case webrtc::PeerConnectionInterface::kIceConnectionClosed:
      ended_ = true;
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
  scoped_refptr<FunctionalSetSessionDescriptionObserver> observer =
      FunctionalSetSessionDescriptionObserver::Create(
          std::bind(
              &P2PPeerConnectionChannel::OnSetLocalSessionDescriptionSuccess,
              this),
          std::bind(
              &P2PPeerConnectionChannel::OnSetLocalSessionDescriptionFailure,
              this, std::placeholders::_1));
  peer_connection_->SetLocalDescription(observer, desc);
}
void P2PPeerConnectionChannel::OnCreateSessionDescriptionFailure(
    const std::string& error) {
  RTC_LOG(LS_INFO) << "Create sdp failed.";
  {
    std::lock_guard<std::mutex> lock(failure_callbacks_mutex_);
    for (std::unordered_map<
             std::string,
             std::function<void(std::unique_ptr<Exception>)>>::iterator it =
             failure_callbacks_.begin();
         it != failure_callbacks_.end(); it++) {
      std::function<void(std::unique_ptr<Exception>)> failure_callback =
          it->second;
      event_queue_->PostTask([failure_callback] {
        std::unique_ptr<Exception> e(new Exception(
            ExceptionType::kP2PClientInvalidArgument, "Failed to create SDP."));
        failure_callback(std::move(e));
      });
    }
    failure_callbacks_.clear();
  }
  Stop(nullptr, nullptr);
}
void P2PPeerConnectionChannel::OnSetLocalSessionDescriptionSuccess() {
  RTC_LOG(LS_INFO) << "Set local sdp success.";
  {
    std::lock_guard<std::mutex> lock(is_creating_offer_mutex_);
    if (is_creating_offer_) {
      is_creating_offer_ = false;
    }
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
  RTC_LOG(LS_INFO) << "Set local sdp failed.";
  {
    std::lock_guard<std::mutex> lock(failure_callbacks_mutex_);
    for (std::unordered_map<
             std::string,
             std::function<void(std::unique_ptr<Exception>)>>::iterator it =
             failure_callbacks_.begin();
         it != failure_callbacks_.end(); it++) {
      std::function<void(std::unique_ptr<Exception>)> failure_callback =
          it->second;
      event_queue_->PostTask([failure_callback] {
        std::unique_ptr<Exception> e(
            new Exception(ExceptionType::kP2PClientInvalidArgument,
                          "Failed to set local SDP."));
        failure_callback(std::move(e));
      });
    }
    failure_callbacks_.clear();
  }
  Stop(nullptr, nullptr);
}
void P2PPeerConnectionChannel::OnSetRemoteSessionDescriptionSuccess() {
  PeerConnectionChannel::OnSetRemoteSessionDescriptionSuccess();
}
void P2PPeerConnectionChannel::OnSetRemoteSessionDescriptionFailure(
    const std::string& error) {
  RTC_LOG(LS_INFO) << "Set remote sdp failed.";
  {
    std::lock_guard<std::mutex> lock(failure_callbacks_mutex_);
    for (std::unordered_map<
             std::string,
             std::function<void(std::unique_ptr<Exception>)>>::iterator it =
             failure_callbacks_.begin();
         it != failure_callbacks_.end(); it++) {
      std::function<void(std::unique_ptr<Exception>)> failure_callback =
          it->second;
      event_queue_->PostTask([failure_callback] {
        std::unique_ptr<Exception> e(
            new Exception(ExceptionType::kP2PClientInvalidArgument,
                          "Failed to set remote SDP."));
        failure_callback(std::move(e));
      });
    }
    failure_callbacks_.clear();
  }
  Stop(nullptr, nullptr);
}
bool P2PPeerConnectionChannel::CheckNullPointer(
    uintptr_t pointer,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  if (pointer)
    return true;
  if (on_failure != nullptr) {
    event_queue_->PostTask([on_failure] {
      std::unique_ptr<Exception> e(new Exception(
          ExceptionType::kP2PClientInvalidArgument, "Nullptr is not allowed."));
      on_failure(std::move(e));
    });
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
  last_disconnect_ = std::chrono::time_point<std::chrono::system_clock>::max();
}
void P2PPeerConnectionChannel::Unpublish(
    std::shared_ptr<LocalStream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  if (!CheckNullPointer((uintptr_t)stream.get(), on_failure)) {
    RTC_LOG(LS_WARNING) << "Local stream cannot be nullptr.";
    return;
  }
  if (!remote_side_supports_remove_stream_) {
    if (on_failure != nullptr) {
      RTC_LOG(LS_WARNING) << "Remote side does not support removeStream.";
      event_queue_->PostTask([on_failure] {
        std::unique_ptr<Exception> e(
            new Exception(ExceptionType::kP2PClientUnsupportedMethod,
                          "Remote side does not support unpublish."));
        on_failure(std::move(e));
      });
    }
    return;
  }
  RTC_CHECK(stream->MediaStream());
  {
    std::lock_guard<std::mutex> lock(published_streams_mutex_);
    auto it = published_streams_.find(stream->MediaStream()->id());
    if (it == published_streams_.end()) {
      if (on_failure) {
        event_queue_->PostTask([on_failure] {
          std::unique_ptr<Exception> e(
              new Exception(ExceptionType::kP2PClientInvalidArgument,
                            "The stream is not published."));
          on_failure(std::move(e));
        });
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
    event_queue_->PostTask([on_success] { on_success(); });
  }
  if (SignalingState() == PeerConnectionInterface::SignalingState::kStable)
    DrainPendingStreams();
}
void P2PPeerConnectionChannel::Stop(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  RTC_LOG(LS_INFO) << "Stop session.";
  switch (session_state_) {
    case kSessionStateConnecting:
    case kSessionStateConnected:
      SendStop(nullptr, nullptr);
      ClosePeerConnection();
      stop_send_needed_ = false;
      ChangeSessionState(kSessionStateReady);
      break;
    case kSessionStateMatched:
      SendStop(nullptr, nullptr);
      stop_send_needed_ = false;
      ChangeSessionState(kSessionStateReady);
      TriggerOnStopped();
      break;
    case kSessionStateOffered:
      SendStop(nullptr, nullptr);
      stop_send_needed_ = false;
      ChangeSessionState(kSessionStateReady);
      TriggerOnStopped();
      break;
    default:
      if (on_failure != nullptr) {
        event_queue_->PostTask([on_failure] {
          std::unique_ptr<Exception> e(
              new Exception(ExceptionType::kP2PClientInvalidState,
                            "Cannot stop a session haven't started."));
          on_failure(std::move(e));
        });
      }
      return;
  }
  if (on_success != nullptr) {
    event_queue_->PostTask([on_success] { on_success(); });
  }
}
void P2PPeerConnectionChannel::GetConnectionStats(
    std::function<void(std::shared_ptr<ConnectionStats>)> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  if (on_success == nullptr) {
    if (on_failure != nullptr) {
      event_queue_->PostTask([on_failure] {
        std::unique_ptr<Exception> e(
            new Exception(ExceptionType::kP2PClientInvalidArgument,
                          "on_success cannot be nullptr. Please provide "
                          "on_success to get connection stats data."));
        on_failure(std::move(e));
      });
    }
    return;
  }
  rtc::scoped_refptr<FunctionalStatsObserver> observer =
      FunctionalStatsObserver::Create(std::move(on_success));
  peer_connection_->GetStats(
      observer, nullptr,
      webrtc::PeerConnectionInterface::kStatsOutputLevelStandard);
}

void P2PPeerConnectionChannel::GetConnectionStats(
    std::function<void(std::shared_ptr<RTCStatsReport>)> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  if (on_success == nullptr) {
    if (on_failure != nullptr) {
      event_queue_->PostTask([on_failure] {
        std::unique_ptr<Exception> e(
            new Exception(ExceptionType::kP2PClientInvalidArgument,
                          "on_success cannot be nullptr. Please provide "
                          "on_success to get connection stats data."));
        on_failure(std::move(e));
      });
    }
    return;
  }
  rtc::scoped_refptr<FunctionalStandardRTCStatsCollectorCallback> observer =
      FunctionalStandardRTCStatsCollectorCallback::Create(
          std::move(on_success));
  peer_connection_->GetStats(observer);
}

void P2PPeerConnectionChannel::GetStats(
    std::function<void(const webrtc::StatsReports& reports)> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  if (on_success == nullptr) {
    if (on_failure != nullptr) {
      event_queue_->PostTask([on_failure] {
        std::unique_ptr<Exception> e(
            new Exception(ExceptionType::kP2PClientInvalidArgument,
                          "on_success cannot be nullptr. Please provide "
                          "on_success to get connection stats data."));
        on_failure(std::move(e));
      });
    }
    return;
  }
  if (session_state_ != kSessionStateConnected) {
    if (on_failure != nullptr) {
      event_queue_->PostTask([on_failure] {
        std::unique_ptr<Exception> e(
            new Exception(ExceptionType::kP2PClientInvalidState,
                          "Cannot get connection stats in this state. Please "
                          "try it after connection is established."));
        on_failure(std::move(e));
      });
    }
    return;
  }
  RTC_LOG(LS_INFO) << "Get native stats";
  rtc::scoped_refptr<FunctionalNativeStatsObserver> observer =
      FunctionalNativeStatsObserver::Create(std::move(on_success));
  peer_connection_->GetStats(
      observer, nullptr,
      webrtc::PeerConnectionInterface::kStatsOutputLevelStandard);
}
bool P2PPeerConnectionChannel::HaveLocalOffer() {
  return SignalingState() == webrtc::PeerConnectionInterface::kHaveLocalOffer;
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
void P2PPeerConnectionChannel::DrainPendingStreams() {
  RTC_LOG(LS_INFO) << "Draining pending stream";
  ChangeSessionState(kSessionStateConnecting);
  bool negotiation_needed = false;
  // First to publish the pending_publish_streams_ list.
  {
    std::lock_guard<std::mutex> lock(pending_publish_streams_mutex_);
    for (auto it = pending_publish_streams_.begin();
         it != pending_publish_streams_.end(); ++it) {
      std::shared_ptr<LocalStream> stream = *it;
      std::string audio_track_source = "mic";
      std::string video_track_source = "camera";
      if (stream->Source().audio == owt::base::AudioSourceInfo::kScreenCast) {
        audio_track_source = "screen-cast";
      }
      if (stream->Source().video == owt::base::VideoSourceInfo::kScreenCast) {
        video_track_source = "screen-cast";
      }
      // Collect stream and tracks information.
      scoped_refptr<webrtc::MediaStreamInterface> media_stream =
          stream->MediaStream();
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
        peer_connection_->AddTrack(track, {media_stream->id()});
      }
      for (const auto& track : media_stream->GetVideoTracks()) {
        // Signaling.
        stream_tracks.append(track->id());
        stream_sources[kStreamVideoSourceKey] = video_track_source;
        track_info[kTrackIdKey] = track->id();
        track_info[kTrackSourceKey] = video_track_source;
        track_sources.append(track_info);
        peer_connection_->AddTrack(track, {media_stream->id()});
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
      negotiation_needed = true;
    }
    pending_publish_streams_.clear();
    if (latest_local_stream_)
      latest_local_stream_ = nullptr;
    if (latest_publish_success_callback_)
      latest_publish_success_callback_ = nullptr;
    if (latest_publish_failure_callback_)
      latest_publish_failure_callback_ = nullptr;
  }
  // Then to unpublish the pending_unpublish_streams_ list.
  {
    std::lock_guard<std::mutex> lock(pending_unpublish_streams_mutex_);
    for (auto it = pending_unpublish_streams_.begin();
         it != pending_unpublish_streams_.end(); ++it) {
      std::shared_ptr<LocalStream> stream = *it;
      scoped_refptr<webrtc::MediaStreamInterface> media_stream =
          stream->MediaStream();
      RTC_CHECK(peer_connection_);
      for (const auto& track : media_stream->GetAudioTracks()) {
        const auto& senders = peer_connection_->GetSenders();
        for (auto& s : senders) {
          const auto& t = s->track();
          if (t != nullptr && t->id() == track->id()) {
            peer_connection_->RemoveTrack(s);
            break;
          }
        }
      }
      for (const auto& track : media_stream->GetVideoTracks()) {
        const auto& senders = peer_connection_->GetSenders();
        for (auto& s : senders) {
          const auto& t = s->track();
          if (t != nullptr && t->id() == track->id()) {
            peer_connection_->RemoveTrack(s);
            break;
          }
        }
      }
      negotiation_needed = true;
    }
    pending_unpublish_streams_.clear();
  }

  if (negotiation_needed) {
    OnNegotiationNeeded();
  }
}
void P2PPeerConnectionChannel::SendStop(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  RTC_LOG(LS_INFO) << "Send stop.";
  Json::Value json;
  json[kMessageTypeKey] = kChatClosed;
  SendSignalingMessage(json, on_success, on_failure);
}
void P2PPeerConnectionChannel::ClosePeerConnection() {
  RTC_LOG(LS_INFO) << "Close peer connection.";
  if (peer_connection_) {
    peer_connection_->Close();
  }
  TriggerOnStopped();
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
  RTC_CHECK(data_channel_);
  if (data_channel_->state() ==
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
  OnNegotiationNeeded();
}
webrtc::DataBuffer P2PPeerConnectionChannel::CreateDataBuffer(
    const std::string& data) {
  rtc::CopyOnWriteBuffer buffer(data.c_str(), data.length());
  webrtc::DataBuffer data_buffer(buffer, false);
  return data_buffer;
}
void P2PPeerConnectionChannel::DrainPendingMessages() {
  RTC_LOG(LS_INFO) << "Draining pending messages. Message queue size: "
                   << pending_messages_.size();
  RTC_CHECK(data_channel_);
  {
    std::lock_guard<std::mutex> lock(pending_messages_mutex_);
    for (auto it = pending_messages_.begin(); it != pending_messages_.end();
         ++it) {
      std::shared_ptr<std::string> message;
      std::function<void()> on_success;
      std::tie(message, on_success, std::ignore)=*it;
      data_channel_->Send(CreateDataBuffer(*message));
      if (on_success) {
        event_queue_->PostTask([on_success]{ on_success(); });
      }
    }
    pending_messages_.clear();
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
  rtc::GetBoolFromJsonObject(capabilities, kUaStreamRemovableKey,
                             &remote_side_supports_remove_stream_);
  rtc::GetBoolFromJsonObject(capabilities, kUaIgnoresDataChannelAcksKey,
                             &remote_side_ignores_datachannel_acks_);
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
