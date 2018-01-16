/*
 * Copyright (c) 2017 Intel Corporation. All rights reserved.
 */

#import "talk/ics/sdk/base/objc/RTCPeerConnectionFactory+ICS.h"
#import "talk/ics/sdk/include/objc/ICS/ICSFactory.h"

@implementation ICSFactory

+ (RTCVideoSource*)videoSource {
  RTCPeerConnectionFactory* pc_factor =
      [RTCPeerConnectionFactory sharedInstance];
  return [pc_factor videoSource];
}

@end
