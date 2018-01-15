/*
 * Copyright (c) 2017 Intel Corporation. All rights reserved.
 */

#import "talk/ics/sdk/base/objc/RTCPeerConnectionFactory+Woogeen.h"
#import "talk/ics/sdk/include/objc/Woogeen/RTCFactory.h"

@implementation RTCFactory

+ (RTCVideoSource*)videoSource {
  RTCPeerConnectionFactory* pc_factor =
      [RTCPeerConnectionFactory sharedInstance];
  return [pc_factor videoSource];
}

@end
