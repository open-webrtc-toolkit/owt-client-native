// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/wasm/rtp_video_receiver.h"
#include <cstdint>
#include "third_party/webrtc/rtc_base/logging.h"

namespace owt {
namespace wasm {
RtpVideoReceiver::RtpVideoReceiver(
    std::unique_ptr<webrtc::RtpVideoStreamReceiver2> receiver)
    : receiver_(std::move(receiver)) {}

bool RtpVideoReceiver::OnRtpPacket(std::vector<uint8_t> packet) {
  RTC_LOG(LS_INFO) << "On RTP packet, size " << packet.size();
  // TODO: Create `rtp_packet_received` from `packet`.
  webrtc::RtpPacketReceived rtp_packet_received;
  receiver_->OnRtpPacket(rtp_packet_received);
  return false;
}
}  // namespace wasm
}  // namespace owt