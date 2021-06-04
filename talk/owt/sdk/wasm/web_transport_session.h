// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_WASM_WEBTRANSPORTSESSION_H_
#define OWT_WASM_WEBTRANSPORTSESSION_H_

#include "third_party/webrtc/api/call/transport.h"

namespace owt {
namespace wasm {
class WebTransportSession : public webrtc::Transport {
 public:
  explicit WebTransportSession() {}
  virtual ~WebTransportSession() {}

  // Overrides webrtc::Transport.
  bool SendRtp(const uint8_t* packet,
               size_t length,
               const webrtc::PacketOptions& options) override;
  bool SendRtcp(const uint8_t* packet, size_t length) override;
};
}  // namespace wasm
}  // namespace owt

#endif