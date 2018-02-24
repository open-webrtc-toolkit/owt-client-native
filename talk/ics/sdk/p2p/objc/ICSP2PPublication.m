//
//  Copyright (c) 2018 Intel Corporation. All rights reserved.
//

#import <WebRTC/RTCLogging.h>

#import "talk/ics/sdk/p2p/objc/ICSP2PPublication+Private.h"

@implementation ICSP2PPublication

- (instancetype)initWithStop:(void (^)())stopMethod {
  if (self = [super init]) {
    _stopMethod = stopMethod;
  }
  return self;
}

-(void)stop{
  _stopMethod();
}

@end
