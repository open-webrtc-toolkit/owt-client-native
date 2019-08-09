// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <WebRTC/RTCVideoCodecFactory.h>
RTC_OBJC_EXPORT
@interface OWTDefaultVideoEncoderFactory : RTCDefaultVideoEncoderFactory
@property(nonatomic, retain) RTCVideoCodecInfo* preferredCodec;
+ (NSArray<RTCVideoCodecInfo*>*)supportedCodecs;
@end
