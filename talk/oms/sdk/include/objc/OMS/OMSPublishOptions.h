// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <Foundation/Foundation.h>
#import <WebRTC/RTCMacros.h>
#import <OMS/OMSMediaFormat.h>
NS_ASSUME_NONNULL_BEGIN
/// PublishOptions defines options for publishing a OMSLocalStream.
RTC_EXPORT
@interface OMSPublishOptions : NSObject
@property(nonatomic, strong) NSArray<OMSAudioEncodingParameters*>* audio;
@property(nonatomic, strong) NSArray<OMSVideoEncodingParameters*>* video;
@end
NS_ASSUME_NONNULL_END
