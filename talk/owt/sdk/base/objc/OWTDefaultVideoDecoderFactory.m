// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#import "talk/owt/sdk/base/objc/OWTDefaultVideoDecoderFactory.h"
#import "RTCEncodedImage.h"
#import "third_party/webrtc/sdk/objc/components/video_codec/RTCH265ProfileLevelId.h"
#import "third_party/webrtc/sdk/objc/components/video_codec/RTCVideoDecoderH265.h"

@implementation OWTDefaultVideoDecoderFactory
- (id<RTCVideoDecoder>)createDecoder:(RTCVideoCodecInfo*)info {
#if defined(WEBRTC_USE_H265)
  if (@available(iOS 11.0, *)) {
    if ([info.name isEqualToString:kRTCVideoCodecH265Name]) {
      return [[RTCVideoDecoderH265 alloc] init];
    }
  }
#endif
  return [super createDecoder:info];
}
- (NSArray<RTCVideoCodecInfo*>*)supportedCodecs {
  NSMutableArray<RTCVideoCodecInfo*>* codecs =
      [[super supportedCodecs] mutableCopy];
#if defined(WEBRTC_USE_H265)
  if (@available(iOS 11.0, *)) {
    [codecs addObject:[[RTCVideoCodecInfo alloc]
                          initWithName:kRTCVideoCodecH265Name]];
  }
#endif
  return codecs;
}
@end
