// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#import "RTCEncodedImage.h"
#if defined(OWT_USE_H265)
#import "RTCVideoCodecH265.h"
#endif
#import "talk/owt/sdk/base/objc/OWTDefaultVideoEncoderFactory.h"
@implementation OWTDefaultVideoEncoderFactory
@synthesize preferredCodec;
+ (NSArray<RTCVideoCodecInfo*>*)supportedCodecs {
  NSMutableArray<RTCVideoCodecInfo*>* codecs =
      [[RTCDefaultVideoEncoderFactory supportedCodecs] mutableCopy];
#if defined(OWT_USE_H265)
  if (@available(iOS 11.0, *)) {
    [codecs addObject:[[RTCVideoCodecInfo alloc]
                          initWithName:kRTCVideoCodecH265Name]];
  }
#endif
  return codecs;
}
- (id<RTCVideoEncoder>)createEncoder:(RTCVideoCodecInfo*)info {
#if defined(OWT_USE_H265)
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
