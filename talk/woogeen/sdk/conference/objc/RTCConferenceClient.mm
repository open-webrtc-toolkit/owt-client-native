//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <string>
#import "talk/woogeen/sdk/conference/objc/public/RTCConferenceClient.h"
#import "talk/woogeen/sdk/conference/socketsignalingchannel.h"
#import "talk/woogeen/sdk/conference/conferenceclient.h"
#import "talk/woogeen/sdk/conference/conferencesignalingchannelinterface.h"
#import "talk/woogeen/sdk/base/objc/RTCStream+Internal.h"
#import "talk/woogeen/sdk/base/objc/RTCLocalStream+Internal.h"
#import "talk/woogeen/sdk/base/objc/RTCRemoteStream+Internal.h"
#import "talk/woogeen/sdk/conference/objc/ConferenceClientObserverObjcImpl.h"

@implementation RTCConferenceClient{
  std::unique_ptr<woogeen::ConferenceClient> _nativeConferenceClient;
}

-(instancetype)init{
  self=[super init];
  std::unique_ptr<woogeen::ConferenceSignalingChannelInterface> socketSignalingChannel(new woogeen::SocketSignalingChannel());
  std::unique_ptr<woogeen::ConferenceClient> nativeConferenceClient(new woogeen::ConferenceClient(std::move(socketSignalingChannel)));
  _nativeConferenceClient=std::move(nativeConferenceClient);
  return self;
}

-(void)addObserver:(id<RTCConferenceClientObserver>)observer {
  woogeen::ConferenceClientObserver *nb=new woogeen::ConferenceClientObserverObjcImpl(observer);
  std::shared_ptr<woogeen::ConferenceClientObserver> nativeObserver(new woogeen::ConferenceClientObserverObjcImpl(observer));
  _nativeConferenceClient->AddObserver(nativeObserver);
}

-(void)removeObserver:(id<RTCConferenceClientObserver>)observer {
}

-(void)join:(NSString *)token onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError *))onFailure{
  const std::string nativeToken=[token UTF8String];
  _nativeConferenceClient->Join(nativeToken, nullptr, nullptr);
}

-(void)publish:(RTCLocalStream *)stream onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError *))onFailure{
  auto nativeStreamRefPtr=[stream nativeStream];
  std::shared_ptr<woogeen::LocalStream> nativeStream(std::static_pointer_cast<woogeen::LocalStream>(nativeStreamRefPtr));
  _nativeConferenceClient->Publish(nativeStream, nullptr, nullptr);
}

-(void)subscribe:(RTCRemoteStream*)stream onSuccess:(void (^)(RTCRemoteStream*))onSuccess onFailure:(void (^)(NSError *))onFailure{
  auto nativeStreamRefPtr=[stream nativeStream];
  std::shared_ptr<woogeen::RemoteStream> nativeStream(std::static_pointer_cast<woogeen::RemoteStream>(nativeStreamRefPtr));
  _nativeConferenceClient->Subscribe(nativeStream, [=](std::shared_ptr<woogeen::RemoteStream> stream){
    RTCRemoteStream* remote_stream = [[RTCRemoteStream alloc]initWithNativeStream: stream];
    onSuccess(remote_stream);
  }, nullptr);
}

-(void)unpublish:(RTCLocalStream*)stream onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError*))onFailure{
  auto nativeStreamRefPtr=[stream nativeStream];
  std::shared_ptr<woogeen::LocalStream> nativeStream(std::static_pointer_cast<woogeen::LocalStream>(nativeStreamRefPtr));
  _nativeConferenceClient->Unpublish(nativeStream, nullptr, nullptr);
}

@end
