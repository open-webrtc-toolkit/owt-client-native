//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <string>
#import "talk/woogeen/sdk/conference/objc/public/ConferenceClient.h"
#import "talk/woogeen/sdk/conference/socketsignalingchannel.h"
#import "talk/woogeen/sdk/conference/conferenceclient.h"
#import "talk/woogeen/sdk/conference/conferencesignalingchannelinterface.h"
#import "talk/woogeen/sdk/base/objc/RTCLocalStream+Internal.h"

@implementation ConferenceClient{
  std::unique_ptr<woogeen::ConferenceClient> _nativeConferenceClient;
}

-(instancetype)init{
  self=[super init];
  std::unique_ptr<woogeen::ConferenceSignalingChannelInterface> socketSignalingChannel(new woogeen::SocketSignalingChannel());
  std::unique_ptr<woogeen::ConferenceClient> nativeConferenceClient(new woogeen::ConferenceClient(std::move(socketSignalingChannel)));
  _nativeConferenceClient=std::move(nativeConferenceClient);
  return self;
}

-(void)join:(NSString *)token onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError *))onFailure{
  const std::string nativeToken=[token UTF8String];
  _nativeConferenceClient->Join(nativeToken, nullptr, nullptr);
}

-(void)publish:(RTCLocalStream *)stream onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError *))onFailure{
  auto nativeStreamRefPtr=[stream nativeLocalStream];
  std::shared_ptr<woogeen::LocalStream> nativeStream(nativeStreamRefPtr.get());
  _nativeConferenceClient->Publish(nativeStream, nullptr, nullptr);
}

@end
