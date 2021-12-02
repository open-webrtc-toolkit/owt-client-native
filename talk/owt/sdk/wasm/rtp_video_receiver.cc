// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/wasm/rtp_video_receiver.h"
#include <cstdint>
#include "third_party/webrtc/rtc_base/logging.h"

namespace owt {
namespace wasm {

RtpVideoReceiver::RtpVideoReceiver(
    webrtc::TaskQueueBase* current_queue,
    webrtc::Transport* transport,
    const webrtc::VideoReceiveStream::Config* config,
    webrtc::ReceiveStatistics* rtp_receive_statistics,
    webrtc::ProcessThread* process_thread)
    : receiver_(nullptr), complete_frame_callback_(emscripten::val::null()) {
  receiver_ = std::make_unique<webrtc::RtpVideoStreamReceiver2>(
      current_queue, webrtc::Clock::GetRealTimeClock(), transport, nullptr,
      nullptr, config, rtp_receive_statistics, nullptr, nullptr, process_thread,
      this, nullptr, this, nullptr, nullptr);
  webrtc::VideoCodec codec;
  codec.codecType = webrtc::VideoCodecType::kVideoCodecH264;
  // TODO: Assign payload value dynamically according to server side
  // configuration.
  receiver_->AddReceiveCodec(127, codec, std::map<std::string, std::string>(),
                             false);
  receiver_->StartReceive();
}

bool RtpVideoReceiver::OnRtpPacket(uintptr_t packet_ptr, size_t packet_size) {
  const uint8_t* packet = reinterpret_cast<uint8_t*>(packet_ptr);
  // TODO: Create `rtp_packet_received` from `packet`.
  webrtc::RtpPacketReceived rtp_packet_received;
  rtp_packet_received.Parse(packet, packet_size);
  receiver_->OnRtpPacket(rtp_packet_received);
  return false;
}

void RtpVideoReceiver::OnCompleteFrame(
    std::unique_ptr<webrtc::video_coding::EncodedFrame> frame) {
  if(complete_frame_callback_.isNull()){
    RTC_LOG(LS_WARNING) << "No callback registered for complete frames.";
    return;
  }
  emscripten::val buffer(emscripten::typed_memory_view(
      frame->EncodedImage().size(), frame->EncodedImage().data()));
  complete_frame_callback_(buffer);
}

void RtpVideoReceiver::SetCompleteFrameCallback(emscripten::val callback) {
  complete_frame_callback_ = callback;
}

void RtpVideoReceiver::SendNack(const std::vector<uint16_t>& sequence_numbers,
                                bool buffering_allowed) {
  // Only buffering_allowed == true is supported. Same as
  // https://source.chromium.org/chromium/chromium/src/+/main:third_party/webrtc/video/video_receive_stream2.h;l=157;bpv=1;bpt=1
  RTC_DCHECK(buffering_allowed);
  RTC_LOG(LS_INFO)<<"Request packet rtx";
  receiver_->RequestPacketRetransmit(sequence_numbers);
}

}  // namespace wasm
}  // namespace owt