// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_WASM_RTPVIDEORECEIVER_H_
#define OWT_WASM_RTPVIDEORECEIVER_H_

#include "third_party/webrtc/video/rtp_video_stream_receiver2.h"

namespace owt {
namespace wasm {
class RtpVideoReceiver {
 public:
  explicit RtpVideoReceiver(
      std::unique_ptr<webrtc::RtpVideoStreamReceiver2> receiver);
  virtual ~RtpVideoReceiver() {}
  virtual bool OnRtpPacket(std::vector<uint8_t> packet);

 private:
  std::unique_ptr<webrtc::RtpVideoStreamReceiver2> receiver_;
};

}  // namespace wasm
}  // namespace owt

#endif