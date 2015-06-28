/*
 * Intel License
 */

#import "Foundation/Foundation.h"
#import "RTCP2PPeerConnectionChannelObserver.h"
#import "talk/app/webrtc/objc/RTCICEServer+Internal.h"
#import "talk/woogeen/sdk/p2p/objc/RTCP2PPeerConnectionChannel.h"
#import "talk/woogeen/sdk/p2p/objc/P2PPeerConnectionChannelObserverObjcImpl.h"
#import "talk/woogeen/sdk/p2p/objc/RTCP2PSignalingSenderObjcImpl.h"
#import "talk/woogeen/sdk/base/objc/RTCLocalStream+Internal.h"
#import "talk/woogeen/sdk/base/objc/RTCStream+Internal.h"

#include "talk/woogeen/sdk/p2p/p2ppeerconnectionchannel.h"

@implementation RTCP2PPeerConnectionChannel {
  woogeen::P2PPeerConnectionChannel* _nativeChannel;
  NSString* _remoteId;
}

-(instancetype)initWithICEServers:(NSArray*)iceServers localId:(NSString*)localId remoteId:(NSString*)remoteId signalingSender:(id<RTCP2PSignalingSenderProtocol>)signalingSender{
  self=[super init];
  woogeen::P2PSignalingSenderInterface* sender = new woogeen::RTCP2PSignalingSenderObjcImpl(signalingSender);
  _remoteId=remoteId;
  const std::string nativeRemoteId=[remoteId UTF8String];
  const std::string nativeLocalId=[localId UTF8String];
  webrtc::PeerConnectionInterface::IceServers nativeIceServers;
  for (RTCICEServer* server in iceServers) {
    nativeIceServers.push_back(server.iceServer);
  }
  NSLog(@"Init P2P peerconnection ice size: %lu", nativeIceServers.size());
  LOG(LS_INFO) << "Init P2P peerconnectionchannel ice size: "<<nativeIceServers.size();
  webrtc::PeerConnectionInterface::RTCConfiguration* config = new webrtc::PeerConnectionInterface::RTCConfiguration();
  config->servers=nativeIceServers;
  _nativeChannel = new woogeen::P2PPeerConnectionChannel(*config, nativeLocalId, nativeRemoteId, sender);
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
  NSLog(@"RTCP2PPeerConnectionChannel publish stream.");
  _nativeChannel->Publish(std::static_pointer_cast<woogeen::LocalStream>([stream nativeStream]), nullptr, nullptr);
}

-(void)unpublish:(RTCLocalStream*)stream onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError *))onFailure{
  NSLog(@"RTCP2PPeerConnectionChannel unpublish stream.");
  _nativeChannel->Unpublish(std::static_pointer_cast<woogeen::LocalStream>([stream nativeStream]), nullptr, nullptr);
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
