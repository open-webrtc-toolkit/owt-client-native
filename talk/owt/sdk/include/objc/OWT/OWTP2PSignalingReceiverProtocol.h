// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <Foundation/Foundation.h>
/// @cond
// RTCSignalingReceiverInterface is an ObjectiveC wrapper for
// SignalingReceiverInterface.
@protocol OWTP2PSignalingReceiverProtocol<NSObject>
- (void)onIncomingSignalingMessage:(NSString*)message;
@end
/// @endcond
