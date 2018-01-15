//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/include/objc/Woogeen/RTCConferenceClientConfiguration.h"

@implementation RTCConferenceClientConfiguration

- (instancetype)init {
  self = [super init];
  _ICEServers = [[NSMutableArray alloc] init];
  return self;
}

@end
