//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "talk/ics/sdk/include/objc/ICS/ICSPeerClientConfiguration.h"

@implementation ICSPeerClientConfiguration

- (instancetype)init {
  self = [super init];
  _ICEServers = [[NSMutableArray alloc] init];
  return self;
}

@end
