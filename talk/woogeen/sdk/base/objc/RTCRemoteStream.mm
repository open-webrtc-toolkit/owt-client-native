//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "talk/app/webrtc/objc/RTCMediaStream+Internal.h"
#import "talk/woogeen/sdk/base/objc/public/RTCRemoteStream.h"
#import "talk/woogeen/sdk/base/objc/RTCRemoteStream+Internal.h"

@implementation RTCRemoteStream {
  rtc::scoped_refptr<woogeen::RemoteStream> _nativeRemoteStream;
}

@end


@implementation RTCRemoteStream (Internal)

-(void)setNativeRemoteStream:(rtc::scoped_refptr<woogeen::RemoteStream>)stream {
  NSLog(@"Set native remote stream.");
  _nativeRemoteStream=stream;
}

-(rtc::scoped_refptr<woogeen::RemoteStream>)nativeRemoteStream{
  return _nativeRemoteStream;
}

-(instancetype)initWithNativeRemoteStream:(rtc::scoped_refptr<woogeen::RemoteStream>)nativeStream{
  NSLog(@"Init with native remote stream");
  self=[super init];
  [self setNativeRemoteStream: nativeStream];
  [super setMediaStream:[[RTCMediaStream alloc]initWithMediaStream:nativeStream->MediaStream()]];
  return self;
}

@end
