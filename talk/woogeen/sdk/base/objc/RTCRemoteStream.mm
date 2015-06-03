//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "talk/app/webrtc/objc/RTCMediaStream+Internal.h"
#import "talk/woogeen/sdk/base/objc/public/RTCRemoteStream.h"
#import "talk/woogeen/sdk/base/objc/RTCRemoteStream+Internal.h"

@implementation RTCRemoteStream {
  std::shared_ptr<woogeen::RemoteStream> _nativeRemoteStream;
}

@end


@implementation RTCRemoteStream (Internal)

-(void)setNativeRemoteStream:(std::shared_ptr<woogeen::RemoteStream>)stream {
  NSLog(@"Set native remote stream.");
  _nativeRemoteStream=stream;
  if(stream->MediaStream()!=nullptr){
    [super setMediaStream:[[RTCMediaStream alloc]initWithMediaStream:stream->MediaStream()]];
  }
}

-(std::shared_ptr<woogeen::RemoteStream>)nativeRemoteStream{
  return _nativeRemoteStream;
}

-(instancetype)initWithNativeRemoteStream:(std::shared_ptr<woogeen::RemoteStream>)nativeStream{
  NSLog(@"Init with native remote stream");
  self=[super init];
  [self setNativeRemoteStream: nativeStream];
  return self;
}

@end
