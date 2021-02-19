// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#import "RTCEncodedImage.h"
#if defined(OWT_USE_H265)
#import "RTCVideoCodecH265.h"
#endif
#import "talk/owt/sdk/base/objc/OWTDefaultVideoDecoderFactory.h"
@implementation OWTDefaultVideoDecoderFactory
- (id<RTCVideoDecoder>)createDecoder:(RTCVideoCodecInfo*)info {
#if defined(OWT_USE_H265)
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
#if defined(OWT_USE_H265)
  if (@available(iOS 11.0, *)) {
    [codecs addObject:[[RTCVideoCodecInfo alloc]
                          initWithName:kRTCVideoCodecH265Name]];
  }
#endif
  return codecs;
}
@end
