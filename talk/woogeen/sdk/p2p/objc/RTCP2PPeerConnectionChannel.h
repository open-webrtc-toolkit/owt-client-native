/*
 * Intel License
 */

#import <Foundation/Foundation.h>

#import "RTCLocalStream.h"
#import "RTCSignalingSenderProtocol.h"
#import "RTCSignalingReceiverProtocol.h"
#import "RTCP2PPeerConnectionChannelObserver.h"

@interface RTCP2PPeerConnectionChannel : NSObject<RTCSignalingReceiverProtocol>

-(instancetype)initWithLocalId:(NSString*)localId remoteId:(NSString*)remoteId signalingSender:(id<RTCSignalingSenderProtocol>)signalingSender;
-(void)inviteWithOnSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError *))onFailure;
-(void)denyWithOnSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError *))onFailure;
-(void)publish:(RTCLocalStream*)stream onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError *))onFailure;
-(void)unpublish:(RTCLocalStream*)stream onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError *))onFailure;
-(void)stopWithOnSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError *))onFailure;
-(void)addObserver:(id<RTCP2PPeerConnectionChannelObserver>)observer;
-(void)removeObserver:(id<RTCP2PPeerConnectionChannelObserver>)observer;
-(NSString*)getRemoteUserId;

@end
