//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "talk/ics/sdk/include/objc/ICS/ICSP2PClientConfiguration.h"

@implementation ICSP2PClientConfiguration

- (instancetype)init {
  self = [super init];
  _ICEServers = [[NSMutableArray alloc] init];
  return self;
}

@end
