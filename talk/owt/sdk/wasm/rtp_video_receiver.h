// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_WASM_RTPVIDEORECEIVER_H_
#define OWT_WASM_RTPVIDEORECEIVER_H_

#include <emscripten/bind.h>
#include "third_party/webrtc/video/rtp_video_stream_receiver2.h"

namespace owt {
namespace wasm {
class RtpVideoReceiver
    : public webrtc::RtpVideoStreamReceiver2::OnCompleteFrameCallback {
 public:
  explicit RtpVideoReceiver(webrtc::Transport* transport,
                            const webrtc::VideoReceiveStream::Config* config,
                            webrtc::ReceiveStatistics* rtp_receive_statistics,
                            webrtc::ProcessThread* process_thread,
                            webrtc::NackSender* nack_sender);
  virtual ~RtpVideoReceiver() {}
  virtual bool OnRtpPacket(uintptr_t packet_ptr, size_t packet_size);

  // Overrides webrtc::RtpVideoStreamReceiver2::OnCompleteFrameCallback.
  void OnCompleteFrame(
      std::unique_ptr<webrtc::video_coding::EncodedFrame> frame) override;

  void SetCompleteFrameCallback(emscripten::val callback);

 private:
  std::unique_ptr<webrtc::RtpVideoStreamReceiver2> receiver_;
  emscripten::val complete_frame_callback_;
};

}  // namespace wasm
}  // namespace owt

#endif