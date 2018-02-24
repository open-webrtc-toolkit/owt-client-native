//
//  Copyright (c) 2018 Intel Corporation. All rights reserved.
//

#import <WebRTC/RTCLogging.h>

#import "talk/ics/sdk/include/objc/ICS/ICSP2PPublication.h"

@interface ICSP2PPublication()

@property(nonatomic, readonly) void (^stopMethod)();

-(instancetype)initWithStop:(void(^)())stopMethod;

@end
