/*
 * Intel License
 */

#import <Foundation/Foundation.h>

#import "talk/woogeen/sdk/base/objc/RTCSignalingSenderProtocol.h"
#import "talk/woogeen/sdk/base/objc/RTCSignalingReceiverProtocol.h"
#import "talk/woogeen/sdk/base/objc/RTCPeerConnectionDependencyFactory.h"
#import "talk/woogeen/sdk/base/objc/public/RTCLocalStream.h"
#import "talk/woogeen/sdk/p2p/objc/RTCP2PPeerConnectionChannelObserver.h"

@interface RTCP2PPeerConnectionChannel : NSObject<RTCSignalingReceiverProtocol>

-(instancetype)initWithLocalId:(NSString*)localId remoteId:(NSString*)remoteId signalingSender:(id<RTCSignalingSenderProtocol>)signalingSender;
-(void)inviteWithOnSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError *))onFailure;
-(void)publish:(RTCLocalStream*)stream onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError *))onFailure;
-(void)addObserver:(id<RTCP2PPeerConnectionChannelObserver>)observer;
-(void)removeObserver:(id<RTCP2PPeerConnectionChannelObserver>)observer;
-(NSString*)getRemoteUserId;

@end
