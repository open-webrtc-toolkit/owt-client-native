//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "talk/app/webrtc/objc/public/RTCVideoTrack.h"
#import "talk/app/webrtc/objc/RTCMediaStream+Internal.h"
#import "talk/woogeen/sdk/base/objc/public/RTCStream.h"
#import "talk/woogeen/sdk/base/objc/RTCStream+Internal.h"

@implementation RTCStream {
  std::shared_ptr<woogeen::base::Stream> _nativeStream;
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
      [[RTCMediaStream alloc] initWithMediaStream:nativeStream->MediaStream()];
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

@end

@implementation RTCStream (Internal)

- (instancetype)initWithNativeStream:(std::shared_ptr<woogeen::base::Stream>)stream {
  self = [super init];
  [self setNativeStream:stream];
  return self;
}

- (void)setNativeStream:(std::shared_ptr<woogeen::base::Stream>)stream {
  _nativeStream = stream;
}

- (std::shared_ptr<woogeen::base::Stream>)nativeStream {
  return _nativeStream;
}

@end
