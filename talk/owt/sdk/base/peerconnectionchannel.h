// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef WOOGEEN_BASE_PEERCONNECTIONCHANNEL_H_
#define WOOGEEN_BASE_PEERCONNECTIONCHANNEL_H_
#include <vector>
#include "webrtc/rtc_base/third_party/sigslot/sigslot.h"
#include "webrtc/sdk/media_constraints.h"
#include "talk/owt/sdk/base/peerconnectiondependencyfactory.h"
#include "talk/owt/sdk/base/functionalobserver.h"
#include "talk/owt/sdk/include/cpp/owt/base/commontypes.h"
namespace rtc {
class NetworkMonitorInterface;
}
namespace owt {
namespace base {
struct PeerConnectionChannelConfiguration
    : public webrtc::PeerConnectionInterface::RTCConfiguration {
 public:
  explicit PeerConnectionChannelConfiguration();
  std::vector<VideoEncodingParameters> video;
  std::vector<AudioEncodingParameters> audio;
  /// Indicate whether this PeerConnection is used for sending encoded frame.
  bool encoded_video_frame_;
};
class PeerConnectionChannel : public webrtc::PeerConnectionObserver,
                              public webrtc::DataChannelObserver,
                              public sigslot::has_slots<> {
 public:
  PeerConnectionChannel(PeerConnectionChannelConfiguration configuration);
 protected:
  virtual ~PeerConnectionChannel();
  bool InitializePeerConnection();
  const webrtc::SessionDescriptionInterface* LocalDescription();
  webrtc::PeerConnectionInterface::SignalingState SignalingState() const;
  // Apply the bitrate settings on all tracks available. Failing to set any of them
  // will result in a false return, with remaining settings applicable still applied.
  // Subclasses can override this to implementation specific bitrate allocation policies.
  void ApplyBitrateSettings();
  // Subclasses should prepare observers for these functions and post
  // message to PeerConnectionChannel.
  virtual void CreateOffer() = 0;
  virtual void CreateAnswer() = 0;
  virtual webrtc::RTCErrorOr<
      rtc::scoped_refptr<webrtc::RtpTransceiverInterface>>
  AddTransceiver(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track,
                 const webrtc::RtpTransceiverInit& init);
  virtual webrtc::RTCErrorOr<
      rtc::scoped_refptr<webrtc::RtpTransceiverInterface>>
  AddTransceiver(cricket::MediaType media_type,
                 const webrtc::RtpTransceiverInit& init);
  // PeerConnectionObserver
  virtual void OnStateChange(webrtc::StatsReport::StatsType state_changed) {}
  virtual void OnSignalingChange(
      webrtc::PeerConnectionInterface::SignalingState new_state) override;
  virtual void OnAddStream(
      rtc::scoped_refptr<MediaStreamInterface> stream) override;
  virtual void OnRemoveStream(
      rtc::scoped_refptr<MediaStreamInterface> stream) override;
  virtual void OnDataChannel(
      rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) override;
  virtual void OnRenegotiationNeeded() override;
  virtual void OnIceConnectionChange(
      webrtc::PeerConnectionInterface::IceConnectionState new_state) override;
  virtual void OnIceGatheringChange(
      webrtc::PeerConnectionInterface::IceGatheringState new_state) override;
  virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
  virtual void OnIceCandidatesRemoved(
      const std::vector<cricket::Candidate>& candidates) override;
  // DataChannelObserver proxy
  // Data channel events will be bridged to these methods to avoid name
  // conflict.
  virtual void OnDataChannelStateChange(){}
  virtual void OnDataChannelMessage(const webrtc::DataBuffer& buffer){}
  // CreateSessionDescriptionObserver
  virtual void OnCreateSessionDescriptionSuccess(
      webrtc::SessionDescriptionInterface* desc);
  virtual void OnCreateSessionDescriptionFailure(const std::string& error);
  // SetSessionDescriptionObserver
  virtual void OnSetLocalSessionDescriptionSuccess();
  virtual void OnSetLocalSessionDescriptionFailure(const std::string& error);
  virtual void OnSetRemoteSessionDescriptionSuccess();
  virtual void OnSetRemoteSessionDescriptionFailure(const std::string& error);
  virtual void OnSetRemoteDescriptionComplete(webrtc::RTCError error);
  // Fired when networks changed. (Only works on iOS)
  virtual void OnNetworksChanged();
  PeerConnectionChannelConfiguration configuration_;
  // Use this data channel to send p2p messages.
  // Use a map if we need more than one data channels for a PeerConnection in
  // the future.
  webrtc::MediaConstraints media_constraints_;
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
  // Direction of audio and video transceivers. In conference mode, there are at
  // most 1 audio transceiver and 1 video transceiver.
  webrtc::RtpTransceiverDirection audio_transceiver_direction_;
  webrtc::RtpTransceiverDirection video_transceiver_direction_;
 private:
  // DataChannelObserver
  virtual void OnStateChange() override { OnDataChannelStateChange(); }
  virtual void OnMessage(const webrtc::DataBuffer& buffer) override {
    OnDataChannelMessage(buffer);
  }
  // |factory_| is got from PeerConnectionDependencyFactory::Get() which is
  // shared among all PeerConnectionChannels.
  rtc::scoped_refptr<PeerConnectionDependencyFactory> factory_;
};
}
}
#endif  // OWT_BASE_PEERCONNECTIONCHANNEL_H_
