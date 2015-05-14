//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "talk/woogeen/sdk/base/objc/public/RTCStream.h"
#import "RTCVideoTrack.h"

@implementation RTCStream

-(instancetype)init{
  return self;
}

-(void)attach:(NSObject<RTCVideoRenderer>*)renderer{
  if(_mediaStream==nil){
    NSLog(@"attached stream without media stream");
    return;
  }
  if([_mediaStream.videoTracks count]==0)
    return;
  [[_mediaStream.videoTracks objectAtIndex:0] addRenderer:renderer];
  NSLog(@"Attach");
}

-(void)setMediaStream:(RTCMediaStream*)stream{
  NSLog(@"set media stream");
  _mediaStream = stream;
}

@end
