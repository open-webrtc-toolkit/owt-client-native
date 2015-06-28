//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#ifndef WOOGEEN_CONFERENCE_OBJC_RTCCONFERENCECLIENTCONFIGURATION_H_
#define WOOGEEN_CONFERENCE_OBJC_RTCCONFERENCECLIENTCONFIGURATION_H_

#import <Foundation/Foundation.h>

// Configuration for creating a RTCConferenceClient
@interface RTCConferenceClientConfiguration : NSObject

@property (nonatomic, strong, readwrite) NSArray* ICEServers;

@end

#endif  // WOOGEEN_CONFERENCE_OBJC_RTCCONFERENCECLIENTCONFIGURATION_H_
