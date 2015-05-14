//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import "RTCVideoCapturer.h"
#import "talk/app/webrtc/objc/RTCVideoCapturer+Internal.h"
#import "talk/app/webrtc/objc/RTCMediaStream+Internal.h"
#import "RTCMediaConstraints.h"
#import "talk/woogeen/sdk/base/objc/public/RTCLocalCameraStream.h"
#import "talk/woogeen/sdk/base/objc/RTCLocalStream+Internal.h"
#import "talk/woogeen/sdk/base/objc/RTCPeerConnectionDependencyFactory.h"

#include "talk/woogeen/sdk/base/stream.h"

@implementation RTCLocalCameraStream

-(id)init{
  self=[super init];
  NSString *cameraId=nil;
  for (AVCaptureDevice *captureDevice in [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo]) {
    if(captureDevice.position==AVCaptureDevicePositionFront){
      cameraId=[captureDevice localizedName];
      break;
    }
  }
  RTCVideoCapturer *capturer=[RTCVideoCapturer capturerWithDeviceName:cameraId];
  NSAssert(cameraId, @"Unable to get the camera ID");
  rtc::scoped_refptr<woogeen::LocalCameraStream> local_stream = woogeen::LocalCameraStream::Create([capturer takeNativeCapturer]);
  [super setNativeLocalStream: local_stream];
  // consider double reference.
  rtc::scoped_refptr<webrtc::MediaStreamInterface> native_media_stream = super.nativeLocalStream->MediaStream();
  super.mediaStream = [[RTCMediaStream alloc] initWithMediaStream:native_media_stream];
  return self;
}

@end

