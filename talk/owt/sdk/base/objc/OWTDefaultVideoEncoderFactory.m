// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#import "talk/owt/sdk/base/objc/OWTDefaultVideoEncoderFactory.h"
#import "RTCEncodedImage.h"
#import "third_party/webrtc/sdk/objc/components/video_codec/RTCH265ProfileLevelId.h"
#import "third_party/webrtc/sdk/objc/components/video_codec/RTCVideoEncoderH265.h"

@implementation OWTDefaultVideoEncoderFactory
@synthesize preferredCodec;
+ (NSArray<RTCVideoCodecInfo*>*)supportedCodecs {
  NSMutableArray<RTCVideoCodecInfo*>* codecs =
      [[RTCDefaultVideoEncoderFactory supportedCodecs] mutableCopy];
#if defined(WEBRTC_USE_H265)
  if (@available(iOS 11.0, *)) {
    [codecs addObject:[[RTCVideoCodecInfo alloc]
                          initWithName:kRTCVideoCodecH265Name]];
  }
#endif
  return codecs;
}
- (id<RTCVideoEncoder>)createEncoder:(RTCVideoCodecInfo*)info {
#if defined(WEBRTC_USE_H265)
  if (@available(iOS 11.0, *)) {
    if ([info.name isEqualToString:kRTCVideoCodecH265Name]) {
      return [[RTCVideoEncoderH265 alloc] initWithCodecInfo:info];
    }
  }
#endif
  return [super createEncoder:info];
}
- (NSArray<RTCVideoCodecInfo*>*)supportedCodecs {
  return [super supportedCodecs];
}
@end
