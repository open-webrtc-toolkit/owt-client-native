/*
 * Intel License
 */

#import <Foundation/Foundation.h>

#include "talk/woogeen/sdk/base/objc/RTCSignalingSenderProtocol.h"
#include "talk/woogeen/sdk/base/objc/RTCSignalingReceiverProtocol.h"
#include "talk/woogeen/sdk/base/objc/RTCPeerConnectionDependencyFactory.h"
#include "talk/woogeen/sdk/p2p/objc/RTCP2PPeerConnectionChannelObserver.h"

@interface RTCP2PPeerConnectionChannel : NSObject<RTCSignalingReceiverProtocol>

-(instancetype)initWithRemoteId:(NSString*)remoteId signalingSender:(id<RTCSignalingSenderProtocol>)signalingSender;
-(void)inviteWithSuccess:(void (^)())success failure:(void (^)(NSError *))failure;
-(void)addObserver:(id<RTCP2PPeerConnectionChannelObserver>)observer;
-(void)removeObserver:(id<RTCP2PPeerConnectionChannelObserver>)observer;
-(NSString*)getRemoteUserId;

@end
