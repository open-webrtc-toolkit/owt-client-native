// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/wasm/rtp_video_receiver.h"
#include <emscripten/threading.h>
#include <cstdint>
#include "third_party/webrtc/rtc_base/location.h"
#include "third_party/webrtc/rtc_base/logging.h"

namespace owt {
namespace wasm {

RtpVideoReceiver::RtpVideoReceiver(
    rtc::Thread* worker_thread,
    webrtc::Transport* transport,
    const webrtc::VideoReceiveStream::Config* config,
    webrtc::ReceiveStatistics* rtp_receive_statistics,
    webrtc::ProcessThread* process_thread)
    : worker_thread_(worker_thread),
      receiver_(nullptr),
      complete_frame_callback_(emscripten::val::null()) {
  receiver_ = std::make_unique<webrtc::RtpVideoStreamReceiver2>(
      worker_thread, webrtc::Clock::GetRealTimeClock(), transport, nullptr,
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

void RtpVideoReceiver::OnRtpPacket(uintptr_t packet_ptr, size_t packet_size) {
  worker_thread_->Invoke<void>(RTC_FROM_HERE, [&] {
    const uint8_t* packet = reinterpret_cast<uint8_t*>(packet_ptr);
    // TODO: Create `rtp_packet_received` from `packet`.
    webrtc::RtpPacketReceived rtp_packet_received;
    rtp_packet_received.Parse(packet, packet_size);
    receiver_->OnRtpPacket(rtp_packet_received);
  });
}

void RtpVideoReceiver::RunCompleteFrameCallback(
    RtpVideoReceiver* receiver,
    uint8_t* buffer,
    size_t size) {
  emscripten::val value(emscripten::typed_memory_view(size, buffer));
  receiver->complete_frame_callback_(value);
}

void RtpVideoReceiver::OnCompleteFrame(
    std::unique_ptr<webrtc::video_coding::EncodedFrame> frame) {
  if (complete_frame_callback_.isNull()) {
    RTC_LOG(LS_WARNING) << "No callback registered for complete frames.";
    return;
  }
  emscripten_sync_run_in_main_runtime_thread(
      EM_FUNC_SIG_VIII, RtpVideoReceiver::RunCompleteFrameCallback, this,
      frame->EncodedImage().data(), frame->EncodedImage().size());
}

void RtpVideoReceiver::SetCompleteFrameCallback(emscripten::val callback) {
  complete_frame_callback_ = callback;
}

void RtpVideoReceiver::SendNack(const std::vector<uint16_t>& sequence_numbers,
                                bool buffering_allowed) {
  // Only buffering_allowed == true is supported. Same as
  // https://source.chromium.org/chromium/chromium/src/+/main:third_party/webrtc/video/video_receive_stream2.h;l=157;bpv=1;bpt=1
  RTC_DCHECK(buffering_allowed);
  receiver_->RequestPacketRetransmit(sequence_numbers);
}

}  // namespace wasm
}  // namespace owt