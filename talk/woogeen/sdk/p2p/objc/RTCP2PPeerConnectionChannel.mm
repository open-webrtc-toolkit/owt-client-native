/*
 * Intel License
 */

#import "talk/woogeen/sdk/p2p/objc/RTCP2PPeerConnectionChannel.h"

#include "talk/woogeen/sdk/p2p/p2ppeerconnectionchannel.h"
#include "talk/woogeen/sdk/base/objc/RTCSignalingSenderObjcImpl.h"

@implementation RTCP2PPeerConnectionChannel {
  woogeen::P2PPeerConnectionChannel* _nativeChannel;
}

-(instancetype)initWithRemoteId:(NSString*)remoteId signalingSender:(id<RTCSignalingSenderProtocol>)signalingSender{
  self=[super init];
  woogeen::SignalingSenderInterface* sender = new woogeen::RTCSignalingSenderObjcImpl(signalingSender);
  const std::string nativeRemoteId=[remoteId UTF8String];
  _nativeChannel = new woogeen::P2PPeerConnectionChannel(nativeRemoteId, sender);
  return self;
}

-(void)inviteWithSuccess:(void (^)())success failure:(void (^)(NSError *))failure{
  _nativeChannel->Invite(nullptr,nullptr);
}

@end
