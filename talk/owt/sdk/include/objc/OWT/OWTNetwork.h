// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <Foundation/Foundation.h>
#import <WebRTC/RTCMacros.h>
/// Define ICE candidate types.
typedef NS_ENUM(NSUInteger, RTCIceCandidateType) {
  /// Host candidate.
  RTCIceCandidateTypeHost = 1,
  /// Server reflexive candidate.
  RTCIceCandidateTypeSrflx,
  /// Peer reflexive candidate.
  RTCIceCandidateTypePrflx,
  /// Relayed candidate.
  RTCIceCandidateTypeRelay,
  /// Unknown.
  RTCIceCandidateTypeUnknown = 99,
};
/// Defines transport protocol.
typedef NS_ENUM(NSUInteger, RTCTransportProtocolType) {
  /// TCP.
  RTCTransportProtocolTypeTcp = 1,
  /// UDP.
  RTCTransportProtocolTypeUdp,
  /// Unknown.
  RTCTransportProtocolTypeUnknown = 99,
};
