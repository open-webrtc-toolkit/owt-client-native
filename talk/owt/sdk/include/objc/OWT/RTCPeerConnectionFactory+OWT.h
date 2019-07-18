// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "WebRTC/RTCPeerConnection.h"
#import "WebRTC/RTCPeerConnectionFactory.h"

@interface RTCPeerConnectionFactory (OWT)

+ (RTCPeerConnectionFactory*)sharedInstance;

@end
