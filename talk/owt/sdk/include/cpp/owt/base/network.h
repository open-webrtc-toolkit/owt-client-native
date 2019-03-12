// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_NETWORK_H_
#define OWT_BASE_NETWORK_H_
namespace owt {
namespace base {
/// Defines ICE server.
struct IceServer {
  /// URLs for this group of ICE server.
  std::vector<std::string> urls;
  /// Username.
  std::string username;
  /// Password.
  std::string password;
};
/// Defines ICE candidate types.
enum class IceCandidateType : int {
  /// Host candidate.
  kHost = 1,
  /// Server reflexive candidate.
  kSrflx,
  /// Peer reflexive candidate.
  kPrflx,
  /// Relayed candidate.
  kRelay,
  /// Unknown.
  kUnknown = 99,
};
/// Defines transport protocol.
enum class TransportProtocolType : int {
  /// TCP.
  kTcp = 1,
  /// UDP.
  kUdp,
  /// Unknown.
  kUnknown=99,
};
}
}
#endif  // OWT_BASE_NETWORK_H_
