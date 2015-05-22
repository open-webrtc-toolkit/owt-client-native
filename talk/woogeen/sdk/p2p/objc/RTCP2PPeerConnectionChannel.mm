/*
 * Intel License
 */

#import "Foundation/Foundation.h"
#import "talk/woogeen/sdk/p2p/objc/RTCP2PPeerConnectionChannel.h"
#import "talk/woogeen/sdk/p2p/objc/RTCP2PPeerConnectionChannelObserver.h"
#import "talk/woogeen/sdk/p2p/objc/P2PPeerConnectionChannelObserverObjcImpl.h"
#import "talk/woogeen/sdk/base/objc/RTCSignalingSenderObjcImpl.h"
#import "talk/woogeen/sdk/base/objc/RTCLocalStream+Internal.h"

#include "talk/woogeen/sdk/p2p/p2ppeerconnectionchannel.h"

@implementation RTCP2PPeerConnectionChannel {
  woogeen::P2PPeerConnectionChannel* _nativeChannel;
  NSString* _remoteId;
}

-(instancetype)initWithLocalId:(NSString*)localId remoteId:(NSString*)remoteId signalingSender:(id<RTCSignalingSenderProtocol>)signalingSender{
  self=[super init];
  woogeen::SignalingSenderInterface* sender = new woogeen::RTCSignalingSenderObjcImpl(signalingSender);
  _remoteId=remoteId;
  const std::string nativeRemoteId=[remoteId UTF8String];
  const std::string nativeLocalId=[localId UTF8String];
  _nativeChannel = new woogeen::P2PPeerConnectionChannel(nativeLocalId, nativeRemoteId, sender);
  return self;
}

-(void)inviteWithOnSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError *))onFailure{
  // TODO(jianjun):correct nullptr.
  _nativeChannel->Invite(nullptr,nullptr);
}

-(void)denyWithOnSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError *))onFailure{
  // TODO(jianjun):correct nullptr.
  _nativeChannel->Deny(nullptr,nullptr);
}

-(void)publish:(RTCLocalStream*)stream onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError *))onFailure{
  _nativeChannel->Publish([stream nativeLocalStream], nullptr, nullptr);
}

-(void)unpublish:(RTCLocalStream*)stream onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError *))onFailure{
  NSLog(@"RTCP2PPeerConnectionChannel unpublish stream.");
  _nativeChannel->Unpublish([stream nativeLocalStream], nullptr, nullptr);
}

-(void)stopWithOnSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError *))onFailure {
  _nativeChannel->Stop(nullptr, nullptr);
}

-(void)onIncomingSignalingMessage:(NSString *)message{
  _nativeChannel->OnIncomingSignalingMessage([message UTF8String]);
}

-(void)addObserver:(id<RTCP2PPeerConnectionChannelObserver>)observer{
  woogeen::P2PPeerConnectionChannelObserver* nativeObserver=new woogeen::P2PPeerConnectionChannelObserverObjcImpl(observer);
  _nativeChannel->AddObserver(nativeObserver);
}

-(void)removeObserver:(id<RTCP2PPeerConnectionChannelObserver>)observer{
}

-(NSString*)getRemoteUserId{
  return _remoteId;
}

@end
