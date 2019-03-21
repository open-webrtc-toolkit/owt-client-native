// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <Foundation/Foundation.h>
@interface OWTP2PPeerConnectionChannelConfiguration : NSObject
@property(strong, nonatomic) RTCICEServer* iceServers;
@property(strong, nonatomic) MediaCodec* mediaCodec;
- (instancetype)init;
@end
