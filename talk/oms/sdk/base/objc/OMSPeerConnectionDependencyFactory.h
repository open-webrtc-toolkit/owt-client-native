// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#import <Foundation/Foundation.h>
#import "WebRTC/RTCMediaStream.h"
@class RTCMediaConstraints;
@interface OMSPeerConnectionDependencyFactory : NSObject
+ (id)sharedOMSPeerConnectionDependencyFactory;
- (RTCMediaStream*)localMediaStreamWithLabel:(NSString*)label;
@end
