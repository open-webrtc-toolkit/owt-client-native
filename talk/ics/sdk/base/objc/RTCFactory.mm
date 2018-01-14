/*
 * Copyright (c) 2017 Intel Corporation. All rights reserved.
 */

#import "talk/woogeen/sdk/base/objc/RTCPeerConnectionFactory+Woogeen.h"
#import "talk/woogeen/sdk/include/objc/Woogeen/RTCFactory.h"

@implementation RTCFactory

+ (RTCVideoSource*)videoSource {
  RTCPeerConnectionFactory* pc_factor =
      [RTCPeerConnectionFactory sharedInstance];
  return [pc_factor videoSource];
}

@end
