/*
 * Intel License
 */

#import <Foundation/Foundation.h>


@interface RTCP2PPeerConnectionChannelConfiguration : NSObject

@property (strong, nonatomic) RTCICEServer *iceServers;
@property (strong, nonatomic) MediaCodec* mediaCodec;

-(instancetype)init;

@end
