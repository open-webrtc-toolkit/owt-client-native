// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <Foundation/Foundation.h>
#import <WebRTC/RTCVideoTrack.h>
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
#import "webrtc/sdk/objc/Framework/Classes/PeerConnection/RTCMediaStream+Private.h"
#import "talk/oms/sdk/include/objc/OMS/OMSStream.h"
#import "talk/oms/sdk/include/objc/OMS/RTCPeerConnectionFactory+OMS.h"
#import "talk/oms/sdk/base/objc/OMSStream+Private.h"
#import "talk/oms/sdk/base/objc/OMSMediaFormat+Private.h"
@implementation OMSStream {
  std::shared_ptr<oms::base::Stream> _nativeStream;
  RTCMediaStream* _mediaStream;
}
- (instancetype)init {
  return self;
}
- (void)attach:(NSObject<RTCVideoRenderer>*)renderer {
  auto nativeStream = [self nativeStream];
  if (nativeStream == nullptr || nativeStream->MediaStream() == nullptr) {
    NSLog(@"Attached stream without media stream");
    return;
  }
  if (_mediaStream == nil) {
    _mediaStream = [[RTCMediaStream alloc]
          initWithFactory:[RTCPeerConnectionFactory sharedInstance]
        nativeMediaStream:nativeStream->MediaStream()];
  }
  if ([_mediaStream.videoTracks count] == 0)
    return;
  [[_mediaStream.videoTracks objectAtIndex:0] addRenderer:renderer];
  NSLog(@"Attached stream.");
}
- (NSDictionary<NSString*, NSString*>*)attributes {
  NSMutableDictionary<NSString*, NSString*>* attrs =
      [NSMutableDictionary dictionary];
  if (_nativeStream == nullptr) {
    return attrs;
  }
  for (auto const& attribute : _nativeStream->Attributes()) {
    [attrs setObject:[NSString stringForStdString:attribute.second]
              forKey:[NSString stringForStdString:attribute.first]];
  }
  return attrs;
}
- (instancetype)initWithNativeStream:
    (std::shared_ptr<oms::base::Stream>)stream {
  self = [super init];
  [self setNativeStream:stream];
  oms::base::StreamSourceInfo* nativeSource =
      new oms::base::StreamSourceInfo(stream->Source());
  std::shared_ptr<oms::base::StreamSourceInfo> sharedNativeSource =
      std::shared_ptr<oms::base::StreamSourceInfo>(nativeSource);
  _source = [[OMSStreamSourceInfo alloc]
      initWithNativeStreamSourceInfo:sharedNativeSource];
  return self;
}
- (instancetype)initWithMediaStream:(RTCMediaStream*)mediaStream
                             source:(OMSStreamSourceInfo*)source {
  if (self = [super init]) {
    _mediaStream = mediaStream;
    _source = source;
  }
  return self;
}
- (void)setNativeStream:(std::shared_ptr<oms::base::Stream>)stream {
  _nativeStream = stream;
}
- (std::shared_ptr<oms::base::Stream>)nativeStream {
  return _nativeStream;
}
@end
