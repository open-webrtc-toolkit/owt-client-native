//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/include/objc/ICS/ICSConferenceClientConfiguration.h"

@implementation ICSConferenceClientConfiguration

- (instancetype)init {
  self = [super init];
  _ICEServers = [[NSMutableArray alloc] init];
  return self;
}

@end
