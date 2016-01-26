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
#import "talk/woogeen/sdk/conference/ConferenceSocketSignalingChannel.h"
#import "talk/woogeen/sdk/conference/objc/ConferenceClientObserverObjcImpl.h"
#import "talk/woogeen/sdk/conference/objc/RTCConferenceSubscribeOptions+Internal.h"
#import "talk/woogeen/sdk/conference/objc/RTCConferenceUser+Internal.h"
#import "talk/woogeen/sdk/include/cpp/woogeen/conference/conferenceclient.h"

@implementation RTCConferenceClient {
  std::unique_ptr<woogeen::conference::ConferenceClient> _nativeConferenceClient;
  std::vector<woogeen::conference::ConferenceClientObserverObjcImpl*> _observers;
}

- (instancetype)initWithConfiguration:
    (RTCConferenceClientConfiguration*)config {
  self = [super init];
  woogeen::conference::ConferenceClientConfiguration* nativeConfig =
      new woogeen::conference::ConferenceClientConfiguration();
  std::vector<woogeen::base::IceServer> iceServers;
  for (RTCICEServer* server in config.ICEServers) {
    woogeen::base::IceServer iceServer;
    iceServer.urls.push_back(server.iceServer.uri);
    iceServer.username=server.iceServer.username;
    iceServer.password=server.iceServer.password;
    iceServers.push_back(iceServer);
  }
  nativeConfig->ice_servers = iceServers;
  LOG(LS_INFO) << "Video codec preference: " << config.mediaCodec.videoCodec;
  if (config.mediaCodec.videoCodec == VideoCodecVP8) {
    nativeConfig->media_codec.video_codec =
        woogeen::base::MediaCodec::VideoCodec::VP8;
  }
  std::unique_ptr<woogeen::conference::ConferenceClient> nativeConferenceClient(
      new woogeen::conference::ConferenceClient(*nativeConfig));
  _nativeConferenceClient = std::move(nativeConferenceClient);
  return self;
}

- (void)addObserver:(id<RTCConferenceClientObserver>)observer {
  auto ob = new woogeen::conference::ConferenceClientObserverObjcImpl(observer);
  _observers.push_back(ob);
  _nativeConferenceClient->AddObserver(*ob);
}

- (void)triggerOnFailure:(void (^)(NSError*))onFailure
           withException:
               (std::unique_ptr<woogeen::conference::ConferenceException>)e {
  if (onFailure == nil)
    return;
  NSError* err = [[NSError alloc]
      initWithDomain:RTCErrorDomain
                code:WoogeenConferenceErrorUnknown
            userInfo:[[NSDictionary alloc]
                         initWithObjectsAndKeys:
                             [NSString
                                 stringWithCString:e->Message().c_str()
                                          encoding:[NSString
                                                       defaultCStringEncoding]],
                             NSLocalizedDescriptionKey, nil]];
  onFailure(err);
}

- (void)removeObserver:(id<RTCConferenceClientObserver>)observer {
}

- (void)joinWithToken:(NSString*)token
            onSuccess:(void (^)(RTCConferenceUser*))onSuccess
            onFailure:(void (^)(NSError*))onFailure {
  const std::string nativeToken = [token UTF8String];
  _nativeConferenceClient->Join(
      nativeToken,
      [=](std::shared_ptr<woogeen::conference::User> user) {
        if (onSuccess != nil)
          onSuccess([[RTCConferenceUser alloc]initWithNativeUser: user]);
      },
      [=](std::unique_ptr<woogeen::conference::ConferenceException> e) {
        [self triggerOnFailure:onFailure withException:(std::move(e))];
      });
}

- (void)publish:(RTCLocalStream*)stream
      onSuccess:(void (^)())onSuccess
      onFailure:(void (^)(NSError*))onFailure {
  auto nativeStreamRefPtr = [stream nativeStream];
  std::shared_ptr<woogeen::base::LocalStream> nativeStream(
      std::static_pointer_cast<woogeen::base::LocalStream>(nativeStreamRefPtr));
  _nativeConferenceClient->Publish(
      nativeStream,
      [=]() {
        if (onSuccess != nil)
          onSuccess();
      },
      [=](std::unique_ptr<woogeen::conference::ConferenceException> e) {
        [self triggerOnFailure:onFailure withException:(std::move(e))];
      });
}

- (void)subscribe:(RTCRemoteStream*)stream
        onSuccess:(void (^)(RTCRemoteStream*))onSuccess
        onFailure:(void (^)(NSError*))onFailure {
  [self subscribe:stream
      withOptions:nil
        onSuccess:onSuccess
        onFailure:onFailure];
}

- (void)subscribe:(RTCRemoteStream*)stream
      withOptions:(RTCConferenceSubscribeOptions*)options
        onSuccess:(void (^)(RTCRemoteStream*))onSuccess
        onFailure:(void (^)(NSError*))onFailure {
  if (options == nil) {
    options = [[RTCConferenceSubscribeOptions alloc] init];
  }
  auto nativeStreamRefPtr = [stream nativeStream];
  std::shared_ptr<woogeen::base::RemoteStream> nativeStream(
      std::static_pointer_cast<woogeen::base::RemoteStream>(nativeStreamRefPtr));
  _nativeConferenceClient->Subscribe(
      nativeStream, [options nativeSubscribeOptions],
      [=](std::shared_ptr<woogeen::base::RemoteStream> stream) {
        RTCRemoteStream* remote_stream =
            [[RTCRemoteStream alloc] initWithNativeStream:stream];
        onSuccess(remote_stream);
      },
      [=](std::unique_ptr<woogeen::conference::ConferenceException> e) {
        [self triggerOnFailure:onFailure withException:(std::move(e))];
      });
}

- (void)unpublish:(RTCLocalStream*)stream
        onSuccess:(void (^)())onSuccess
        onFailure:(void (^)(NSError*))onFailure {
  auto nativeStreamRefPtr = [stream nativeStream];
  std::shared_ptr<woogeen::base::LocalStream> nativeStream(
      std::static_pointer_cast<woogeen::base::LocalStream>(nativeStreamRefPtr));
  _nativeConferenceClient->Unpublish(
      nativeStream,
      [=]() {
        if (onSuccess != nil)
          onSuccess();
      },
      [=](std::unique_ptr<woogeen::conference::ConferenceException> e) {
        [self triggerOnFailure:onFailure withException:(std::move(e))];
      });
}

- (void)unsubscribe:(RTCRemoteStream*)stream
          onSuccess:(void (^)())onSuccess
          onFailure:(void (^)(NSError*))onFailure {
  auto nativeStreamRefPtr = [stream nativeStream];
  std::shared_ptr<woogeen::base::RemoteStream> nativeStream(
      std::static_pointer_cast<woogeen::base::RemoteStream>(nativeStreamRefPtr));
  _nativeConferenceClient->Unsubscribe(
      nativeStream,
      [=]() {
        if (onSuccess != nil)
          onSuccess();
      },
      [=](std::unique_ptr<woogeen::conference::ConferenceException> e) {
        [self triggerOnFailure:onFailure withException:(std::move(e))];
      });
}

- (void)send:(NSString*)message
   onSuccess:(void (^)())onSuccess
   onFailure:(void (^)(NSError*))onFailure {
  _nativeConferenceClient->Send(
      [message UTF8String],
      [=] {
        if (onSuccess != nil)
          onSuccess();
      },
      [=](std::unique_ptr<woogeen::conference::ConferenceException> e) {
        [self triggerOnFailure:onFailure withException:(std::move(e))];
      });
}

- (void)playAudio:(RTCStream*)stream
        onSuccess:(void (^)())onSuccess
        onFailure:(void (^)(NSError*))onFailure {
  auto nativeStreamRefPtr = [stream nativeStream];
  std::shared_ptr<woogeen::base::Stream> nativeStream(
      std::static_pointer_cast<woogeen::base::Stream>(nativeStreamRefPtr));
  _nativeConferenceClient->PlayAudio(
      nativeStream,
      [=]() {
        if (onSuccess != nil)
          onSuccess();
      },
      [=](std::unique_ptr<woogeen::conference::ConferenceException> e) {
        [self triggerOnFailure:onFailure withException:(std::move(e))];
      });
}

- (void)pauseAudio:(RTCStream*)stream
         onSuccess:(void (^)())onSuccess
         onFailure:(void (^)(NSError*))onFailure {
  auto nativeStreamRefPtr = [stream nativeStream];
  std::shared_ptr<woogeen::base::Stream> nativeStream(
      std::static_pointer_cast<woogeen::base::Stream>(nativeStreamRefPtr));
  _nativeConferenceClient->PauseAudio(
      nativeStream,
      [=]() {
        if (onSuccess != nil)
          onSuccess();
      },
      [=](std::unique_ptr<woogeen::conference::ConferenceException> e) {
        [self triggerOnFailure:onFailure withException:(std::move(e))];
      });
}

- (void)playVideo:(RTCStream*)stream
        onSuccess:(void (^)())onSuccess
        onFailure:(void (^)(NSError*))onFailure {
  auto nativeStreamRefPtr = [stream nativeStream];
  std::shared_ptr<woogeen::base::Stream> nativeStream(
      std::static_pointer_cast<woogeen::base::Stream>(nativeStreamRefPtr));
  _nativeConferenceClient->PlayVideo(
      nativeStream,
      [=]() {
        if (onSuccess != nil)
          onSuccess();
      },
      [=](std::unique_ptr<woogeen::conference::ConferenceException> e) {
        [self triggerOnFailure:onFailure withException:(std::move(e))];
      });
}

- (void)pauseVideo:(RTCStream*)stream
         onSuccess:(void (^)())onSuccess
         onFailure:(void (^)(NSError*))onFailure {
  auto nativeStreamRefPtr = [stream nativeStream];
  std::shared_ptr<woogeen::base::Stream> nativeStream(
      std::static_pointer_cast<woogeen::base::Stream>(nativeStreamRefPtr));
  _nativeConferenceClient->PauseVideo(
      nativeStream,
      [=]() {
        if (onSuccess != nil)
          onSuccess();
      },
      [=](std::unique_ptr<woogeen::conference::ConferenceException> e) {
        [self triggerOnFailure:onFailure withException:(std::move(e))];
      });
}

- (void)leaveWithOnSuccess:(void (^)())onSuccess
                 onFailure:(void (^)(NSError*))onFailure {
  _nativeConferenceClient->Leave(
      [=]() {
        if (onSuccess != nil)
          onSuccess();
      },
      [=](std::unique_ptr<woogeen::conference::ConferenceException> e) {
        [self triggerOnFailure:onFailure withException:(std::move(e))];
      });
}

- (void)getRegion:(RTCRemoteStream*)stream
        onSuccess:(void (^)(NSString*))onSuccess
        onFailure:(void (^)(NSError*))onFailure {
  auto nativeStreamRefPtr = [stream nativeStream];
  std::shared_ptr<woogeen::base::RemoteStream> nativeStream(
      std::static_pointer_cast<woogeen::base::RemoteStream>(
          nativeStreamRefPtr));
  _nativeConferenceClient->GetRegion(
      nativeStream,
      [=](std::string region_id) {
        if (onSuccess) {
          onSuccess([NSString stringWithUTF8String:region_id.c_str()]);
        }
      },
      [=](std::unique_ptr<woogeen::conference::ConferenceException> e) {
        [self triggerOnFailure:onFailure withException:(std::move(e))];
      });
}

- (void)setRegion:(RTCRemoteStream*)stream
         regionId:(NSString*)regionId
        onSuccess:(void (^)())onSuccess
        onFailure:(void (^)(NSError*))onFailure{
  auto nativeStreamRefPtr = [stream nativeStream];
  std::shared_ptr<woogeen::base::RemoteStream> nativeStream(
      std::static_pointer_cast<woogeen::base::RemoteStream>(
          nativeStreamRefPtr));
  auto nativeRegionid = [regionId UTF8String];
  _nativeConferenceClient->SetRegion(
      nativeStream, nativeRegionid,
      [=]() {
        if (onSuccess) {
          onSuccess();
        }
      },
      [=](std::unique_ptr<woogeen::conference::ConferenceException> e) {
        [self triggerOnFailure:onFailure withException:(std::move(e))];
      });
}

- (void)dealloc {
  while(!_observers.empty())
    delete _observers.back(), _observers.pop_back();
}

@end
