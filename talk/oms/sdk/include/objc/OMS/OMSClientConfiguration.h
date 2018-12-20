// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <WebRTC/RTCConfiguration.h>
NS_ASSUME_NONNULL_BEGIN
@class OMSClientConfiguration;
/// Base class for configurations for conference and P2P client.
RTC_EXPORT
@interface OMSClientConfiguration : NSObject
/// Configuration for WebRTC connections.
@property(nonatomic, strong) RTCConfiguration* rtcConfiguration;
@end
NS_ASSUME_NONNULL_END
