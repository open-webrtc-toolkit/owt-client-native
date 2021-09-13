// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_WASM_MEDIASESSION_H_
#define OWT_WASM_MEDIASESSION_H_

#include "call/call.h"
#include "talk/owt/sdk/wasm/rtp_video_receiver.h"
#include "talk/owt/sdk/wasm/web_transport_session.h"

namespace owt {
namespace wasm {
// Manages a media session between a web client and OWT server QUIC agent.
class MediaSession {
 public:
  explicit MediaSession();
  virtual ~MediaSession();
  RtpVideoReceiver* CreateRtpVideoReceiver();

 private:
  std::unique_ptr<WebTransportSession> web_transport_session_;
  std::unique_ptr<webrtc::Call> call_;
  std::unique_ptr<RtpVideoReceiver> video_receiver_;
};

}  // namespace wasm
}  // namespace owt

#endif