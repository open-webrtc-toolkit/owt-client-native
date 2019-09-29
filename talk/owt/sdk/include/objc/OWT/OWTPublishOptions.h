// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <Foundation/Foundation.h>
#import <WebRTC/RTCMacros.h>
#import <OWT/OWTMediaFormat.h>
NS_ASSUME_NONNULL_BEGIN
/// PublishOptions defines options for publishing a OWTLocalStream.
RTC_OBJC_EXPORT
@interface OWTPublishOptions : NSObject
@property(nonatomic, strong) NSArray<OWTAudioEncodingParameters*>* audio;
@property(nonatomic, strong) NSArray<OWTVideoEncodingParameters*>* video;
@end
NS_ASSUME_NONNULL_END
