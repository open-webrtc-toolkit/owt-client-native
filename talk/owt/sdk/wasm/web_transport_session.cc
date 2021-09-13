// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/wasm/web_transport_session.h"

namespace owt {
namespace wasm {
  
bool WebTransportSession::SendRtp(const uint8_t* packet,
                                  size_t length,
                                  const webrtc::PacketOptions& options) {
  return false;
}
bool WebTransportSession::SendRtcp(const uint8_t* packet, size_t length) {
  return false;
}

}  // namespace wasm
}  // namespace owt