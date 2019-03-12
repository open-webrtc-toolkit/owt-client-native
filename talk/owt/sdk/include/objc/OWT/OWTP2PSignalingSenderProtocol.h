// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <Foundation/Foundation.h>
/// @cond
// RTCSignalingSenderInterface is an ObjectiveC wrapper for
// SignalingSenderInterface.
@protocol OWTP2PSignalingSenderProtocol<NSObject>
- (void)sendSignalingMessage:(NSString*)message
                          to:(NSString*)targetId
                   onSuccess:(void (^)())onSuccess
                   onFailure:(void (^)(NSError*))onFailure;
@end
/// @endcond
