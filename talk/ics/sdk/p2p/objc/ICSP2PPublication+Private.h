//
//  Copyright (c) 2018 Intel Corporation. All rights reserved.
//

#import <WebRTC/RTCLegacyStatsReport.h>
#import <WebRTC/RTCLogging.h>

#import "talk/ics/sdk/include/objc/ICS/ICSP2PPublication.h"

@interface ICSP2PPublication ()

@property(nonatomic, readonly) void (^stopMethod)();
@property(nonatomic, readonly) void (^statsMethod)
    (void (^)(NSArray<RTCLegacyStatsReport*>*), void (^)(NSError*));

- (instancetype)initWithStop:(void (^)())stopMethod
                       stats:
                           (void (^)(void (^)(NSArray<RTCLegacyStatsReport*>*),
                                     void (^)(NSError*)))statsMethod;

@end
