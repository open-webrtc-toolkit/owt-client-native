// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#import <Foundation/Foundation.h>
#import "RTCMediaStream.h"
@class RTCMediaConstraints;
@interface OWTPeerConnectionDependencyFactory : NSObject
+ (id)sharedOWTPeerConnectionDependencyFactory;
- (RTCMediaStream*)localMediaStreamWithLabel:(NSString*)label;
@end
