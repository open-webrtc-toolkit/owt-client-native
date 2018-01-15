//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <WebRTC/RTCVideoTrack.h>
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
#import "webrtc/sdk/objc/Framework/Classes/PeerConnection/RTCMediaStream+Private.h"
#import "talk/ics/sdk/include/objc/Woogeen/RTCStream.h"
#import "talk/ics/sdk/base/objc/RTCStream+Internal.h"

@implementation RTCStream {
  std::shared_ptr<ics::base::Stream> _nativeStream;
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
  _mediaStream =
      [[RTCMediaStream alloc] initWithNativeMediaStream:nativeStream->MediaStream()];
  if ([_mediaStream.videoTracks count] == 0)
    return;
  [[_mediaStream.videoTracks objectAtIndex:0] addRenderer:renderer];
  NSLog(@"Attached stream.");
}

- (void)disableAudio {
  if (_nativeStream == nullptr)
    return;
  _nativeStream->DisableAudio();
}

- (void)disableVideo {
  if (_nativeStream == nullptr)
    return;
  _nativeStream->DisableVideo();
}

- (void)enableAudio {
  if (_nativeStream == nullptr)
    return;
  _nativeStream->EnableAudio();
}

- (void)enableVideo {
  if (_nativeStream == nullptr)
    return;
  _nativeStream->EnableVideo();
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

@end

@implementation RTCStream (Internal)

- (instancetype)initWithNativeStream:(std::shared_ptr<ics::base::Stream>)stream {
  self = [super init];
  [self setNativeStream:stream];
  return self;
}

- (void)setNativeStream:(std::shared_ptr<ics::base::Stream>)stream {
  _nativeStream = stream;
}

- (std::shared_ptr<ics::base::Stream>)nativeStream {
  return _nativeStream;
}

@end
