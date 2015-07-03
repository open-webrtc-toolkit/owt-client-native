/*
 * Intel License
 */

#import <Foundation/Foundation.h>

#import "RTCLocalStream.h"
#import "RTCP2PSignalingSenderProtocol.h"
#import "RTCP2PSignalingReceiverProtocol.h"
#import "RTCP2PPeerConnectionChannelObserver.h"

@interface RTCP2PPeerConnectionChannel : NSObject<RTCP2PSignalingReceiverProtocol>

-(instancetype)initWithICEServers:(NSArray*)iceServers localId:(NSString*)localId remoteId:(NSString*)remoteId signalingSender:(id<RTCP2PSignalingSenderProtocol>)signalingSender;
-(void)inviteWithOnSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError *))onFailure;
-(void)denyWithOnSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError *))onFailure;
-(void)acceptWithOnSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError *))onFailure;
-(void)publish:(RTCLocalStream*)stream onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError *))onFailure;
-(void)unpublish:(RTCLocalStream*)stream onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError *))onFailure;
-(void)stopWithOnSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError *))onFailure;
-(void)addObserver:(id<RTCP2PPeerConnectionChannelObserver>)observer;
-(void)removeObserver:(id<RTCP2PPeerConnectionChannelObserver>)observer;
-(NSString*)getRemoteUserId;

@end
