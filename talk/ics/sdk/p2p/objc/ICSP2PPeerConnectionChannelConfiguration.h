/*
 * Intel License
 */

#import <Foundation/Foundation.h>

@interface ICSP2PPeerConnectionChannelConfiguration : NSObject

@property(strong, nonatomic) RTCICEServer* iceServers;
@property(strong, nonatomic) MediaCodec* mediaCodec;

- (instancetype)init;

@end
