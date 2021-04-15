// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "talk/owt/sdk/base/peerconnectionchannel.h"
#include <vector>
#include "talk/owt/sdk/base/sdputils.h"
#include "webrtc/api/peer_connection_interface.h"
#include "webrtc/rtc_base/logging.h"
#include "webrtc/rtc_base/thread.h"
#include "webrtc/system_wrappers/include/field_trial.h"

using namespace rtc;
namespace owt {
namespace base {
PeerConnectionChannel::PeerConnectionChannel(
    PeerConnectionChannelConfiguration configuration)
    : configuration_(configuration),
      peer_connection_(nullptr),
      factory_(nullptr) {}

PeerConnectionChannel::~PeerConnectionChannel() {
  if (peer_connection_ != nullptr) {
    peer_connection_->Close();
    peer_connection_ = nullptr;
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
  configuration_.media_config.enable_dscp = true;
  // Johny: This must not be set if we use seperate AV channels.
  if (!webrtc::field_trial::IsEnabled("OWT-IceUnbundle")) {
     configuration_.bundle_policy =
       webrtc::PeerConnectionInterface::BundlePolicy::kBundlePolicyMaxBundle;
  }
  peer_connection_ =
      (factory_->CreatePeerConnection(configuration_, this)).get();
  if (!peer_connection_.get()) {
    RTC_LOG(LS_ERROR) << "Failed to initialize PeerConnection.";
    RTC_DCHECK(false);
    return false;
  }
  RTC_CHECK(peer_connection_);
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

void PeerConnectionChannel::OnSetRemoteDescriptionComplete(
    webrtc::RTCError error) {
  if (error.ok()) {
    OnSetRemoteSessionDescriptionSuccess();
  } else {
    OnSetRemoteSessionDescriptionFailure(error.message());
  }
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
