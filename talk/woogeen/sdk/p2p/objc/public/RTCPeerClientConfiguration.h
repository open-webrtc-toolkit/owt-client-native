//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#ifndef p2p_RTCP2PClientConfiguration_h
#define p2p_RTCP2PClientConfiguration_h

#import <Foundation/Foundation.h>
#import "RTCICEServer.h"
#import "RTCMediaCodec.h"

/**
 @brief Configuration for RTCPeerClient

 This configuration is used while creating RTCPeerClient. Changing this configuration does NOT impact RTCPeerClient already created.
 */
@interface RTCPeerClientConfiguration : NSObject

@property (nonatomic, strong, readwrite) NSArray* ICEServers;
@property (nonatomic, strong, readwrite) RTCMediaCodec* mediaCodec;

@end

#endif
