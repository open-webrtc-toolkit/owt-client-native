// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_WASM_BINDING_H_
#define OWT_WASM_BINDING_H_

#include <emscripten/bind.h>
#include "talk/owt/sdk/wasm/media_session.h"
#include "talk/owt/sdk/wasm/rtp_video_receiver.h"

namespace owt {
namespace wasm {

EMSCRIPTEN_BINDINGS(Owt) {
  emscripten::class_<MediaSession>("MediaSession").constructor<>();
  emscripten::class_<RtpVideoReceiver>("RtpVideoReceiver")
      .function("onRtpPacket", &RtpVideoReceiver::OnRtpPacket);
}

}  // namespace wasm
}  // namespace owt

#endif