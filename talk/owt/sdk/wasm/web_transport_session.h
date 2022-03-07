// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_WASM_WEBTRANSPORTSESSION_H_
#define OWT_WASM_WEBTRANSPORTSESSION_H_

#include "third_party/webrtc/api/call/transport.h"
#include "third_party/webrtc/modules/include/module_common_types.h"

namespace owt {
namespace wasm {

class WebTransportSession : public webrtc::Transport,
                            public webrtc::NackSender {
 public:
  explicit WebTransportSession() {}
  virtual ~WebTransportSession() {}

  // Overrides webrtc::Transport.
  bool SendRtp(const uint8_t* packet,
               size_t length,
               const webrtc::PacketOptions& options) override;
  bool SendRtcp(const uint8_t* packet, size_t length) override;

  // Overrides webrtc::NackSender.
  void SendNack(const std::vector<uint16_t>& sequence_numbers,
                bool buffering_allowed) override;
};

}  // namespace wasm
}  // namespace owt

#endif