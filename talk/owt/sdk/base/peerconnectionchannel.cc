// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "talk/owt/sdk/base/peerconnectionchannel.h"
#include <vector>
#include "talk/owt/sdk/base/sdputils.h"
#include "webrtc/rtc_base/logging.h"
#include "webrtc/rtc_base/thread.h"
using namespace rtc;
namespace owt {
namespace base {
PeerConnectionChannel::PeerConnectionChannel(
    PeerConnectionChannelConfiguration configuration)
    : pc_thread_(nullptr),
      configuration_(configuration),
      factory_(nullptr),
      peer_connection_(nullptr) {}
PeerConnectionChannel::~PeerConnectionChannel() {
  if (peer_connection_ != nullptr) {
    peer_connection_->Close();
  }
}
bool PeerConnectionChannel::InitializePeerConnection() {
  RTC_LOG(LS_INFO) << "Initialize PeerConnection.";
  if (factory_.get() == nullptr)
    factory_ = PeerConnectionDependencyFactory::Get();
  audio_transceiver_direction_ = webrtc::RtpTransceiverDirection::kSendRecv;
  video_transceiver_direction_ = webrtc::RtpTransceiverDirection::kSendRecv;
  configuration_.enable_dtls_srtp = true;
  configuration_.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
  peer_connection_ =
      (factory_->CreatePeerConnection(configuration_, this)).get();
  if (!peer_connection_.get()) {
    RTC_LOG(LS_ERROR) << "Failed to initialize PeerConnection.";
    RTC_DCHECK(false);
    return false;
  }
  if (pc_thread_ == nullptr) {
    pc_thread_ = rtc::Thread::CreateWithSocketServer();
    pc_thread_->SetName("pc_thread", nullptr);
    pc_thread_->Start();
  }
  RTC_CHECK(peer_connection_);
  RTC_CHECK(pc_thread_);
  rtc::NetworkMonitorInterface* network_monitor = factory_->NetworkMonitor();
  if (network_monitor) {
    network_monitor->SignalNetworksChanged.connect(
        this, &PeerConnectionChannel::OnNetworksChanged);
  }
  return true;
}
void PeerConnectionChannel::ApplyBitrateSettings() {
  RTC_CHECK(peer_connection_);
  std::vector<rtc::scoped_refptr<webrtc::RtpSenderInterface>> senders =
      peer_connection_->GetSenders();
  if (senders.size() == 0) {
    RTC_LOG(LS_WARNING) << "Cannot set max bitrate without stream added.";
    return;
  }
  for (auto sender : senders) {
    auto sender_track = sender->track();
    if (sender_track != nullptr) {
      webrtc::RtpParameters rtp_parameters = sender->GetParameters();
      for (size_t idx = 0; idx < rtp_parameters.encodings.size(); idx++) {
        // TODO(jianlin): It may not be appropriate to set the same settings
        // on all encodings. Update the logic when upstream implement the
        // the per codec settings, and many other encoding specific setttings
        // should move here instead of modifying SDP.
        if (sender_track->kind() ==
            webrtc::MediaStreamTrackInterface::kAudioKind) {
          if (configuration_.audio.size() > 0 &&
              configuration_.audio[0].max_bitrate > 0) {
            rtp_parameters.encodings[idx].max_bitrate_bps =
                absl::optional<int>(configuration_.audio[0].max_bitrate * 1024);
            sender->SetParameters(rtp_parameters);
          }
        } else if (sender_track->kind() ==
                   webrtc::MediaStreamTrackInterface::kVideoKind) {
          if (configuration_.video.size() > 0 &&
              configuration_.video[0].max_bitrate > 0) {
            rtp_parameters.encodings[idx].max_bitrate_bps =
                absl::optional<int>(configuration_.video[0].max_bitrate * 1024);
            sender->SetParameters(rtp_parameters);
          }
        }
      }
    }
  }
  return;
}

void PeerConnectionChannel::AddTransceiver(
    rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track,
    const webrtc::RtpTransceiverInit& init) {
  peer_connection_->AddTransceiver(track, init);
}

void PeerConnectionChannel::AddTransceiver(
    cricket::MediaType media_type,
    const webrtc::RtpTransceiverInit& init) {
  peer_connection_->AddTransceiver(media_type, init);
}

const webrtc::SessionDescriptionInterface*
PeerConnectionChannel::LocalDescription() {
  RTC_CHECK(peer_connection_);
  return peer_connection_->local_description();
}
PeerConnectionInterface::SignalingState PeerConnectionChannel::SignalingState()
    const {
  RTC_CHECK(peer_connection_);
  return peer_connection_->signaling_state();
}
void PeerConnectionChannel::ClosePc(){
  pc_thread_->Post(RTC_FROM_HERE, this, kMessageTypeClosePeerConnection, nullptr);
}
void PeerConnectionChannel::OnMessage(rtc::Message* msg) {
  RTC_CHECK(peer_connection_);
  if (peer_connection_->signaling_state() ==
      webrtc::PeerConnectionInterface::SignalingState::kClosed) {
    RTC_LOG(LS_WARNING)
        << "Attempt to perform PeerConnection operation when it is closed.";
    return;
  }
  switch (msg->message_id) {
    case kMessageTypeClosePeerConnection:
      peer_connection_->Close();
      break;
    case kMessageTypeCreateOffer: {
      rtc::TypedMessageData<
          scoped_refptr<FunctionalCreateSessionDescriptionObserver>>* param =
          static_cast<rtc::TypedMessageData<
              scoped_refptr<FunctionalCreateSessionDescriptionObserver>>*>(
              msg->pdata);
      peer_connection_->CreateOffer(
          param->data(),
          webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
      delete param;
      break;
    }
    case kMessageTypeCreateAnswer: {
      rtc::TypedMessageData<
          scoped_refptr<FunctionalCreateSessionDescriptionObserver>>* param =
          static_cast<rtc::TypedMessageData<
              scoped_refptr<FunctionalCreateSessionDescriptionObserver>>*>(
              msg->pdata);
      peer_connection_->CreateAnswer(
          param->data(),
          webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
      delete param;
      break;
    }
    case kMessageTypeCreateDataChannel: {
      rtc::TypedMessageData<std::string>* param =
          static_cast<rtc::TypedMessageData<std::string>*>(msg->pdata);
      webrtc::DataChannelInit config;
      data_channel_ =
          peer_connection_->CreateDataChannel(param->data(), &config);
      data_channel_->RegisterObserver(this);
      RTC_LOG(LS_INFO) << "Created data channel.";
      delete param;
      break;
    }
    case kMessageTypeSetLocalDescription: {
      SetSessionDescriptionMessage* param =
          static_cast<SetSessionDescriptionMessage*>(msg->pdata);
      // Set codec preference.
      webrtc::SessionDescriptionInterface* desc = param->description;
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
      webrtc::SessionDescriptionInterface* new_desc(
          webrtc::CreateSessionDescription(desc->type(), sdp_string, nullptr));
      peer_connection_->SetLocalDescription(param->observer, new_desc);
      delete param;
      break;
    }
    case kMessageTypeSetRemoteDescription: {
      SetSessionDescriptionMessage* param =
          static_cast<SetSessionDescriptionMessage*>(msg->pdata);
      // Set codec preference.
      webrtc::SessionDescriptionInterface* desc = param->description;
      std::string sdp_string;
      if (!desc->ToString(&sdp_string)) {
        RTC_LOG(LS_ERROR) << "Error parsing local description.";
        RTC_DCHECK(false);
      }
      webrtc::SessionDescriptionInterface* new_desc(
          webrtc::CreateSessionDescription(desc->type(), sdp_string, nullptr));
      peer_connection_->SetRemoteDescription(param->observer, new_desc);
      delete param;
      break;
    }
    case kMessageTypeSetRemoteIceCandidate: {
      rtc::TypedMessageData<webrtc::IceCandidateInterface*>* param =
          static_cast<rtc::TypedMessageData<webrtc::IceCandidateInterface*>*>(
              msg->pdata);
      peer_connection_->AddIceCandidate(param->data());
      delete param;
      break;
    }
    case kMessageTypeRemoveStream: {
      rtc::ScopedRefMessageData<MediaStreamInterface>* param =
          static_cast<rtc::ScopedRefMessageData<MediaStreamInterface>*>(
              msg->pdata);
      MediaStreamInterface* stream = param->data();
      for (const auto& track : stream->GetAudioTracks()) {
        RTC_CHECK(peer_connection_);
        std::vector<rtc::scoped_refptr<webrtc::RtpSenderInterface>> senders =
            peer_connection_->GetSenders();
        if (senders.size() == 0) {
          return;
        }
        for (auto sender : senders) {
          auto sender_track = sender->track();
          if (sender_track != nullptr && sender_track->id() == track->id()) {
            peer_connection_->RemoveTrack(sender);
            break;
          }
        }
      }

      for (const auto& track : stream->GetVideoTracks()) {
        RTC_CHECK(peer_connection_);
        std::vector<rtc::scoped_refptr<webrtc::RtpSenderInterface>> senders =
            peer_connection_->GetSenders();
        if (senders.size() == 0) {
          return;
        }
        for (auto sender : senders) {
          auto sender_track = sender->track();
          if (sender_track != nullptr && sender_track->id() == track->id()) {
            peer_connection_->RemoveTrack(sender);
            break;
          }
        }
      }
      delete param;
      break;
    }
    case kMessageTypeGetStats: {
      GetStatsMessage* param = static_cast<GetStatsMessage*>(msg->pdata);
      peer_connection_->GetStats(param->observer, nullptr, param->level);
      delete param;
      break;
    }
    default:
      RTC_LOG(LS_WARNING) << "Unknown message type.";
  }
}
void PeerConnectionChannel::OnSetRemoteSessionDescriptionSuccess() {
  RTC_LOG(LS_INFO) << "Set remote sdp success.";
  if (peer_connection_->remote_description() &&
      peer_connection_->remote_description()->type() == "offer") {
    CreateAnswer();
  }
}
void PeerConnectionChannel::OnSetRemoteSessionDescriptionFailure(
    const std::string& error) {
  RTC_LOG(LS_INFO) << "Set remote sdp failed.";
}
void PeerConnectionChannel::OnCreateSessionDescriptionSuccess(
    webrtc::SessionDescriptionInterface* desc) {
  RTC_LOG(LS_INFO) << "Create sdp success.";
}
void PeerConnectionChannel::OnCreateSessionDescriptionFailure(
    const std::string& error) {
  RTC_LOG(LS_INFO) << "Create sdp failed.";
}
void PeerConnectionChannel::OnSetLocalSessionDescriptionSuccess() {
  RTC_LOG(LS_INFO) << "Set local sdp success.";
}
void PeerConnectionChannel::OnSetLocalSessionDescriptionFailure(
    const std::string& error) {
  RTC_LOG(LS_INFO) << "Set local sdp failed.";
}
void PeerConnectionChannel::OnIceCandidate(
    const webrtc::IceCandidateInterface* candidate) {}
void PeerConnectionChannel::OnIceCandidatesRemoved(
    const std::vector<cricket::Candidate>& candidates) {}
void PeerConnectionChannel::OnSignalingChange(
    PeerConnectionInterface::SignalingState new_state) {}
void PeerConnectionChannel::OnAddStream(
    rtc::scoped_refptr<MediaStreamInterface> stream) {}
void PeerConnectionChannel::OnRemoveStream(
    rtc::scoped_refptr<MediaStreamInterface> stream) {}
void PeerConnectionChannel::OnDataChannel(
    rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) {}
void PeerConnectionChannel::OnRenegotiationNeeded() {}
void PeerConnectionChannel::OnIceConnectionChange(
    PeerConnectionInterface::IceConnectionState new_state) {}
void PeerConnectionChannel::OnIceGatheringChange(
    PeerConnectionInterface::IceGatheringState new_state) {}
void PeerConnectionChannel::OnNetworksChanged() {
  RTC_LOG(LS_INFO) << "PeerConnectionChannel::OnNetworksChanged.";
}
PeerConnectionChannelConfiguration::PeerConnectionChannelConfiguration()
    : RTCConfiguration() {}
}  // namespace base
}  // namespace owt
