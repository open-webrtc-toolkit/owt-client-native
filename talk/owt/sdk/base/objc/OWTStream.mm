// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <Foundation/Foundation.h>
#import <WebRTC/RTCVideoTrack.h>
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
#import "webrtc/sdk/objc/Framework/Classes/PeerConnection/RTCMediaStream+Private.h"
#import "talk/owt/sdk/include/objc/OWT/OWTStream.h"
#import "talk/owt/sdk/include/objc/OWT/RTCPeerConnectionFactory+OWT.h"
#import "talk/owt/sdk/base/objc/OWTStream+Private.h"
#import "talk/owt/sdk/base/objc/OWTMediaFormat+Private.h"
@implementation OWTStream {
  std::shared_ptr<owt::base::Stream> _nativeStream;
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
    (std::shared_ptr<owt::base::Stream>)stream {
  self = [super init];
  [self setNativeStream:stream];
  owt::base::StreamSourceInfo* nativeSource =
      new owt::base::StreamSourceInfo(stream->Source());
  std::shared_ptr<owt::base::StreamSourceInfo> sharedNativeSource =
      std::shared_ptr<owt::base::StreamSourceInfo>(nativeSource);
  _source = [[OWTStreamSourceInfo alloc]
      initWithNativeStreamSourceInfo:sharedNativeSource];
  return self;
}
- (instancetype)initWithMediaStream:(RTCMediaStream*)mediaStream
                             source:(OWTStreamSourceInfo*)source {
  if (self = [super init]) {
    _mediaStream = mediaStream;
    _source = source;
  }
  return self;
}
- (void)setNativeStream:(std::shared_ptr<owt::base::Stream>)stream {
  _nativeStream = stream;
  if (_nativeStream->MediaStream() &&
      _nativeStream->MediaStream() != _mediaStream.nativeMediaStream) {
    _mediaStream = [[RTCMediaStream alloc]
          initWithFactory:[RTCPeerConnectionFactory sharedInstance]
        nativeMediaStream:stream->MediaStream()];
  }
}
- (std::shared_ptr<owt::base::Stream>)nativeStream {
  return _nativeStream;
}
@end
