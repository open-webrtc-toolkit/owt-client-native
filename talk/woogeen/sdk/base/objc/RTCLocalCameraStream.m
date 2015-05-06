//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import "talk/woogeen/sdk/base/objc/public/RTCLocalCameraStream.h"
#import "RTCVideoCapturer.h"
#import "RTCMediaConstraints.h"
#import "talk/woogeen/sdk/base/objc/RTCPeerConnectionDependencyFactory.h"

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
  RTCMediaStream *stream=[[RTCPeerConnectionDependencyFactory sharedRTCPeerConnectionDependencyFactory] localMediaStreamWithLabel:@"ARDAMS" capturer:capturer constraints:[[RTCMediaConstraints alloc] initWithMandatoryConstraints:nil optionalConstraints:nil]];
  super.mediaStream=stream;
  return self;
}

@end
