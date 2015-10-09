//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "RTCPeerClientConfiguration.h"

@implementation RTCPeerClientConfiguration

- (instancetype)init {
  self = [super init];
  _ICEServers = [[NSMutableArray alloc] init];
  _mediaCodec = [[RTCMediaCodec alloc] init];
  _mediaCodec.videoCodec = VideoCodecH264;  // Default video codec: H.264
  return self;
}

@end
