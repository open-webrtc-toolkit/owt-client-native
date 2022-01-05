// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_WASM_BINDING_H_
#define OWT_WASM_BINDING_H_

#include "talk/owt/sdk/wasm/media_session.h"
#include "talk/owt/sdk/wasm/rtp_video_receiver.h"

namespace owt {
namespace wasm {

EMSCRIPTEN_BINDINGS(Owt) {
  emscripten::class_<MediaSession>("MediaSession")
      .smart_ptr<std::shared_ptr<MediaSession>>("MediaSession")
      .function("createRtpVideoReceiver", &MediaSession::CreateRtpVideoReceiver)
      .function("setRtcpCallback", &MediaSession::SetRtcpCallback)
      .class_function("get", &MediaSession::Get,
                      emscripten::allow_raw_pointers());
  emscripten::class_<RtpVideoReceiver>("RtpVideoReceiver")
      .smart_ptr<std::shared_ptr<RtpVideoReceiver>>("RtpVideoReceiver")
      .function("onRtpPacket", &RtpVideoReceiver::OnRtpPacket,
                emscripten::allow_raw_pointers())
      .function("setCompleteFrameCallback",
                &RtpVideoReceiver::SetCompleteFrameCallback);
}

}  // namespace wasm
}  // namespace owt

#endif