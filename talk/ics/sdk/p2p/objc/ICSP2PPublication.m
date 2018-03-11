//
//  Copyright (c) 2018 Intel Corporation. All rights reserved.
//

#import <WebRTC/RTCLogging.h>

#import "talk/ics/sdk/p2p/objc/ICSP2PPublication+Private.h"

@implementation ICSP2PPublication

- (instancetype)initWithStop:(void (^)())stopMethod
                       stats:
                           (void (^)(void (^)(NSArray<RTCLegacyStatsReport*>*),
                                     void (^)(NSError*)))statsMethod {
  if (self = [super init]) {
    _stopMethod = stopMethod;
    _statsMethod = statsMethod;
  }
  return self;
}

- (void)stop {
  _stopMethod();
}

- (void)stats:(void (^)(NSArray<RTCLegacyStatsReport*>* stats))onSuccess
    onFailure:(nullable void (^)(NSError*))onFailure {
  _statsMethod(onSuccess, onFailure);
}

@end
