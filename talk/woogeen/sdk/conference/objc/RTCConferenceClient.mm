//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <string>
#import "talk/app/webrtc/objc/RTCICEServer+Internal.h"
#import "talk/woogeen/sdk/base/objc/RTCStream+Internal.h"
#import "talk/woogeen/sdk/base/objc/RTCLocalStream+Internal.h"
#import "talk/woogeen/sdk/base/objc/RTCRemoteStream+Internal.h"
#import "talk/woogeen/sdk/base/objc/public/RTCErrors.h"
#import "talk/woogeen/sdk/conference/objc/public/RTCConferenceClient.h"
#import "talk/woogeen/sdk/conference/objc/public/RTCConferenceErrors.h"
#import "talk/woogeen/sdk/conference/socketsignalingchannel.h"
#import "talk/woogeen/sdk/conference/conferenceclient.h"
#import "talk/woogeen/sdk/conference/conferencesignalingchannelinterface.h"
#import "talk/woogeen/sdk/conference/objc/ConferenceClientObserverObjcImpl.h"

@implementation RTCConferenceClient{
  std::unique_ptr<woogeen::ConferenceClient> _nativeConferenceClient;
}

-(instancetype)initWithConfiguration:(RTCConferenceClientConfiguration*)config {
  self=[super init];
  woogeen::ConferenceClientConfiguration* nativeConfig = new woogeen::ConferenceClientConfiguration();
  webrtc::PeerConnectionInterface::IceServers iceServers;
  for(RTCICEServer* server in config.ICEServers){
    iceServers.push_back(server.iceServer);
  }
  nativeConfig->ice_servers=iceServers;
  LOG(LS_INFO) << "Video codec preference: "<< config.mediaCodec.videoCodec;
  if(config.mediaCodec.videoCodec==VideoCodecVP8){
    nativeConfig->media_codec.video_codec=woogeen::MediaCodec::VideoCodec::VP8;
  }
  std::shared_ptr<woogeen::ConferenceSignalingChannelInterface> socketSignalingChannel(new woogeen::SocketSignalingChannel());
  std::unique_ptr<woogeen::ConferenceClient> nativeConferenceClient(new woogeen::ConferenceClient(*nativeConfig, socketSignalingChannel));
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

-(void)joinWithOnSuccess:(NSString *)token onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError *))onFailure{
  const std::string nativeToken=[token UTF8String];
  _nativeConferenceClient->Join(nativeToken, [=](){
    if(onSuccess!=nil)
      onSuccess();
  }, [=](std::unique_ptr<woogeen::ConferenceException> e){
    if(onFailure==nil)
      return;
    NSError *err=[[NSError alloc]initWithDomain:RTCErrorDomain code:WoogeenConferenceErrorUnknown userInfo:[[NSDictionary alloc]initWithObjectsAndKeys:[NSString stringWithCString:e->Message().c_str() encoding: [NSString defaultCStringEncoding]], NSLocalizedDescriptionKey, nil]];
    onFailure(err);
  });
}

-(void)publish:(RTCLocalStream *)stream onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError *))onFailure{
  auto nativeStreamRefPtr=[stream nativeStream];
  std::shared_ptr<woogeen::LocalStream> nativeStream(std::static_pointer_cast<woogeen::LocalStream>(nativeStreamRefPtr));
  _nativeConferenceClient->Publish(nativeStream, [=](){
    if(onSuccess!=nil)
      onSuccess();
  }, [=](std::unique_ptr<woogeen::ConferenceException> e){
    if(onFailure==nil)
      return;
    NSError *err=[[NSError alloc]initWithDomain:RTCErrorDomain code:WoogeenConferenceErrorUnknown userInfo:[[NSDictionary alloc]initWithObjectsAndKeys:[NSString stringWithCString:e->Message().c_str() encoding: [NSString defaultCStringEncoding]], NSLocalizedDescriptionKey, nil]];
    onFailure(err);
  });
}

-(void)subscribe:(RTCRemoteStream*)stream onSuccess:(void (^)(RTCRemoteStream*))onSuccess onFailure:(void (^)(NSError *))onFailure{
  auto nativeStreamRefPtr=[stream nativeStream];
  std::shared_ptr<woogeen::RemoteStream> nativeStream(std::static_pointer_cast<woogeen::RemoteStream>(nativeStreamRefPtr));
  _nativeConferenceClient->Subscribe(nativeStream, [=](std::shared_ptr<woogeen::RemoteStream> stream){
    RTCRemoteStream* remote_stream = [[RTCRemoteStream alloc]initWithNativeStream: stream];
    onSuccess(remote_stream);
  }, [=](std::unique_ptr<woogeen::ConferenceException> e){
    if(onFailure==nil)
      return;
    NSError *err=[[NSError alloc]initWithDomain:RTCErrorDomain code:WoogeenConferenceErrorUnknown userInfo:[[NSDictionary alloc]initWithObjectsAndKeys:[NSString stringWithCString:e->Message().c_str() encoding: [NSString defaultCStringEncoding]], NSLocalizedDescriptionKey, nil]];
    onFailure(err);
  });
}

-(void)unpublish:(RTCLocalStream*)stream onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError*))onFailure{
  auto nativeStreamRefPtr=[stream nativeStream];
  std::shared_ptr<woogeen::LocalStream> nativeStream(std::static_pointer_cast<woogeen::LocalStream>(nativeStreamRefPtr));
  _nativeConferenceClient->Unpublish(nativeStream, [=](){
    if(onSuccess!=nil)
      onSuccess();
  }, [=](std::unique_ptr<woogeen::ConferenceException> e){
    if(onFailure==nil)
      return;
    NSError *err=[[NSError alloc]initWithDomain:RTCErrorDomain code:WoogeenConferenceErrorUnknown userInfo:[[NSDictionary alloc]initWithObjectsAndKeys:[NSString stringWithCString:e->Message().c_str() encoding: [NSString defaultCStringEncoding]], NSLocalizedDescriptionKey, nil]];
    onFailure(err);
  });
}

-(void)unsubscribe:(RTCRemoteStream*)stream onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError*))onFailure{
  auto nativeStreamRefPtr=[stream nativeStream];
  std::shared_ptr<woogeen::RemoteStream> nativeStream(std::static_pointer_cast<woogeen::RemoteStream>(nativeStreamRefPtr));
  _nativeConferenceClient->Unsubscribe(nativeStream, [=](){
    if(onSuccess!=nil)
      onSuccess();
  }, [=](std::unique_ptr<woogeen::ConferenceException> e){
    if(onFailure==nil)
      return;
    NSError *err=[[NSError alloc]initWithDomain:RTCErrorDomain code:WoogeenConferenceErrorUnknown userInfo:[[NSDictionary alloc]initWithObjectsAndKeys:[NSString stringWithCString:e->Message().c_str() encoding: [NSString defaultCStringEncoding]], NSLocalizedDescriptionKey, nil]];
    onFailure(err);
  });
}

-(void)send:(NSString*)message onSuccess:(void (^)())onSuccess onFailure:(void(^)(NSError*))onFailure{
  _nativeConferenceClient->Send([message UTF8String], [=]{
    if(onSuccess!=nil)
      onSuccess();
  }, [=](std::unique_ptr<woogeen::ConferenceException> e){
    if(onFailure==nil)
      return;
    NSError *err=[[NSError alloc]initWithDomain:RTCErrorDomain code:WoogeenConferenceErrorUnknown userInfo:[[NSDictionary alloc]initWithObjectsAndKeys:[NSString stringWithCString:e->Message().c_str() encoding: [NSString defaultCStringEncoding]], NSLocalizedDescriptionKey, nil]];
    onFailure(err);
  });
}

-(void)playAudio:(RTCStream*)stream onSuccess:(void (^)())onSuccess onFailure:(void(^)(NSError*))onFailure{
  auto nativeStreamRefPtr=[stream nativeStream];
  std::shared_ptr<woogeen::Stream> nativeStream(std::static_pointer_cast<woogeen::Stream>(nativeStreamRefPtr));
  _nativeConferenceClient->PlayAudio(nativeStream, [=](){
    if(onSuccess!=nil)
      onSuccess();
  }, [=](std::unique_ptr<woogeen::ConferenceException> e){
    if(onFailure==nil)
      return;
    NSError *err=[[NSError alloc]initWithDomain:RTCErrorDomain code:WoogeenConferenceErrorUnknown userInfo:[[NSDictionary alloc]initWithObjectsAndKeys:[NSString stringWithCString:e->Message().c_str() encoding: [NSString defaultCStringEncoding]], NSLocalizedDescriptionKey, nil]];
    onFailure(err);
  });
}

-(void)pauseAudio:(RTCStream*)stream onSuccess:(void (^)())onSuccess onFailure:(void(^)(NSError*))onFailure{
  auto nativeStreamRefPtr=[stream nativeStream];
  std::shared_ptr<woogeen::Stream> nativeStream(std::static_pointer_cast<woogeen::Stream>(nativeStreamRefPtr));
  _nativeConferenceClient->PauseAudio(nativeStream, [=](){
    if(onSuccess!=nil)
      onSuccess();
  }, [=](std::unique_ptr<woogeen::ConferenceException> e){
    if(onFailure==nil)
      return;
    NSError *err=[[NSError alloc]initWithDomain:RTCErrorDomain code:WoogeenConferenceErrorUnknown userInfo:[[NSDictionary alloc]initWithObjectsAndKeys:[NSString stringWithCString:e->Message().c_str() encoding: [NSString defaultCStringEncoding]], NSLocalizedDescriptionKey, nil]];
    onFailure(err);
  });
}

-(void)playVideo:(RTCStream*)stream onSuccess:(void (^)())onSuccess onFailure:(void(^)(NSError*))onFailure{
  auto nativeStreamRefPtr=[stream nativeStream];
  std::shared_ptr<woogeen::Stream> nativeStream(std::static_pointer_cast<woogeen::Stream>(nativeStreamRefPtr));
  _nativeConferenceClient->PlayVideo(nativeStream, [=](){
    if(onSuccess!=nil)
      onSuccess();
  }, [=](std::unique_ptr<woogeen::ConferenceException> e){
    if(onFailure==nil)
      return;
    NSError *err=[[NSError alloc]initWithDomain:RTCErrorDomain code:WoogeenConferenceErrorUnknown userInfo:[[NSDictionary alloc]initWithObjectsAndKeys:[NSString stringWithCString:e->Message().c_str() encoding: [NSString defaultCStringEncoding]], NSLocalizedDescriptionKey, nil]];
    onFailure(err);
  });
}

-(void)pauseVideo:(RTCStream*)stream onSuccess:(void (^)())onSuccess onFailure:(void(^)(NSError*))onFailure{
  auto nativeStreamRefPtr=[stream nativeStream];
  std::shared_ptr<woogeen::Stream> nativeStream(std::static_pointer_cast<woogeen::Stream>(nativeStreamRefPtr));
  _nativeConferenceClient->PauseVideo(nativeStream, [=](){
    if(onSuccess!=nil)
      onSuccess();
  }, [=](std::unique_ptr<woogeen::ConferenceException> e){
    if(onFailure==nil)
      return;
    NSError *err=[[NSError alloc]initWithDomain:RTCErrorDomain code:WoogeenConferenceErrorUnknown userInfo:[[NSDictionary alloc]initWithObjectsAndKeys:[NSString stringWithCString:e->Message().c_str() encoding: [NSString defaultCStringEncoding]], NSLocalizedDescriptionKey, nil]];
    onFailure(err);
  });
}

-(void)leaveWithOnSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError*))onFailure{
  _nativeConferenceClient->Leave([=](){
    if(onSuccess!=nil)
      onSuccess();
  }, [=](std::unique_ptr<woogeen::ConferenceException> e){
    if(onFailure==nil)
      return;
    NSError *err=[[NSError alloc]initWithDomain:RTCErrorDomain code:WoogeenConferenceErrorUnknown userInfo:[[NSDictionary alloc]initWithObjectsAndKeys:[NSString stringWithCString:e->Message().c_str() encoding: [NSString defaultCStringEncoding]], NSLocalizedDescriptionKey, nil]];
    onFailure(err);
  });
}

@end
