//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <WebRTC/RTCMediaConstraints.h>
#import "webrtc/sdk/objc/Framework/Classes/RTCMediaStream+Private.h"
#import "talk/woogeen/sdk/include/objc/Woogeen/RTCLocalCameraStream.h"
#import "talk/woogeen/sdk/base/objc/RTCLocalCameraStreamParameters+Internal.h"
#import "talk/woogeen/sdk/base/objc/RTCStream+Internal.h"
#import "talk/woogeen/sdk/base/objc/RTCLocalStream+Internal.h"
#import "talk/woogeen/sdk/base/objc/RTCPeerConnectionDependencyFactory.h"

#include "talk/woogeen/sdk/include/cpp/woogeen/base/stream.h"

@implementation RTCLocalCameraStream

- (instancetype)initWithParameters:(RTCLocalCameraStreamParameters*)parameters {
  self = [super init];
  woogeen::base::LocalCameraStreamParameters local_parameters =
      *[parameters nativeParameters].get();
  std::shared_ptr<woogeen::base::LocalCameraStream> local_stream =
      std::make_shared<woogeen::base::LocalCameraStream>(local_parameters);
  [super setNativeStream:local_stream];
  return self;
}

@end
