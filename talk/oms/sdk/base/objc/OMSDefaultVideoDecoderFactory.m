#import "WebRTC/RTCVideoCodec.h"
#import "WebRTC/RTCVideoCodecFactory.h"
#if !defined(RTC_DISABLE_H265)
#import "WebRTC/RTCVideoCodecH265.h"
#endif
#import "talk/oms/sdk/base/objc/OMSDefaultVideoDecoderFactory.h"
@implementation OMSDefaultVideoDecoderFactory
- (id<RTCVideoDecoder>)createDecoder:(RTCVideoCodecInfo*)info {
#if !defined(RTC_DISABLE_H265)
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
#if !defined(RTC_DISABLE_H265)
  if (@available(iOS 11.0, *)) {
    [codecs addObject:[[RTCVideoCodecInfo alloc]
                          initWithName:kRTCVideoCodecH265Name]];
  }
#endif
  return codecs;
}
@end
