//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <string>
#import "talk/woogeen/sdk/base/objc/RTCConnectionStats+Internal.h"
#import "talk/woogeen/sdk/base/objc/RTCStream+Internal.h"
#import "talk/woogeen/sdk/base/objc/RTCLocalStream+Internal.h"
#import "talk/woogeen/sdk/base/objc/RTCRemoteStream+Internal.h"
#import "talk/woogeen/sdk/base/objc/RTCMediaCodec+Internal.h"
#import "talk/woogeen/sdk/include/objc/Woogeen/RTCErrors.h"
#import "talk/woogeen/sdk/include/objc/Woogeen/RTCConferenceClient.h"
#import "talk/woogeen/sdk/include/objc/Woogeen/RTCConferenceErrors.h"
#import "talk/woogeen/sdk/conference/conferencesocketsignalingchannel.h"
#import "talk/woogeen/sdk/conference/objc/ConferenceClientObserverObjcImpl.h"
#import "talk/woogeen/sdk/conference/objc/RTCConferenceClient+Internal.h"
#import "talk/woogeen/sdk/conference/objc/RTCConferenceSubscribeOptions+Internal.h"
#import "talk/woogeen/sdk/conference/objc/RTCConferenceUser+Internal.h"
#import "talk/woogeen/sdk/include/cpp/woogeen/conference/conferenceclient.h"
#import "talk/woogeen/sdk/include/cpp/woogeen/conference/remotemixedstream.h"
#import "webrtc/sdk/objc/Framework/Classes/NSString+StdString.h"
#import "webrtc/sdk/objc/Framework/Classes/RTCIceServer+Private.h"

@implementation RTCConferenceClient {
  std::unique_ptr<woogeen::conference::ConferenceClient> _nativeConferenceClient;
  std::vector<woogeen::conference::ConferenceClientObserverObjcImpl*> _observers;
  NSMutableDictionary<NSString*, RTCLocalStream*>* _publishedStreams;
}

- (instancetype)initWithConfiguration:
    (RTCConferenceClientConfiguration*)config {
  self = [super init];
  woogeen::conference::ConferenceClientConfiguration* nativeConfig =
      new woogeen::conference::ConferenceClientConfiguration();
  std::vector<woogeen::base::IceServer> iceServers;
  for (RTCIceServer* server in config.ICEServers) {
    woogeen::base::IceServer iceServer;
    iceServer.urls = server.nativeServer.urls;
    iceServer.username = server.nativeServer.username;
    iceServer.password = server.nativeServer.password;
    iceServers.push_back(iceServer);
  }
  nativeConfig->ice_servers = iceServers;
  nativeConfig->max_audio_bandwidth = [config maxAudioBandwidth];
  nativeConfig->max_video_bandwidth = [config maxVideoBandwidth];
  nativeConfig->media_codec.audio_codec =
      [RTCMediaCodec nativeAudioCodec:config.mediaCodec.audioCodec];
  nativeConfig->media_codec.video_codec =
      [RTCMediaCodec nativeVideoCodec:config.mediaCodec.videoCodec];
  nativeConfig->candidate_network_policy =
      ([config candidateNetworkPolicy] == RTCCandidateNetworkPolicyLowCost)
          ? woogeen::base::ClientConfiguration::CandidateNetworkPolicy::kLowCost
          : woogeen::base::ClientConfiguration::CandidateNetworkPolicy::kAll;
  std::unique_ptr<woogeen::conference::ConferenceClient> nativeConferenceClient(
      new woogeen::conference::ConferenceClient(*nativeConfig));
  _nativeConferenceClient = std::move(nativeConferenceClient);
  _publishedStreams = [[NSMutableDictionary alloc] init];
  return self;
}

- (void)addObserver:(id<RTCConferenceClientObserver>)observer {
  auto ob =
      new woogeen::conference::ConferenceClientObserverObjcImpl(observer, self);
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
                             [NSString stringForStdString:e->Message()],
                             NSLocalizedDescriptionKey, nil]];
  onFailure(err);
}

- (void)removeObserver:(id<RTCConferenceClientObserver>)observer {
}

- (void)joinWithToken:(NSString*)token
            onSuccess:(void (^)(RTCConferenceUser*))onSuccess
            onFailure:(void (^)(NSError*))onFailure {
  if (token == nil) {
    if (onFailure != nil) {
      NSError* err = [[NSError alloc]
          initWithDomain:RTCErrorDomain
                    code:WoogeenConferenceErrorUnknown
                userInfo:[[NSDictionary alloc]
                             initWithObjectsAndKeys:@"Token cannot be nil.",
                                                    NSLocalizedDescriptionKey,
                                                    nil]];
      onFailure(err);
    }
    return;
  }
  const std::string nativeToken = [token UTF8String];
  _nativeConferenceClient->Join(
      nativeToken,
      [=](std::shared_ptr<woogeen::conference::User> user) {
        RTCConferenceUser* conferenceUser =
            [[RTCConferenceUser alloc] initWithNativeUser:user];
        if (onSuccess != nil)
          onSuccess(conferenceUser);
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
        [_publishedStreams setObject:stream forKey:[stream streamId]];
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
  RTCConferenceSubscribeOptions* options =
      [[RTCConferenceSubscribeOptions alloc] init];
  [self subscribe:stream
      withOptions:options
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

- (void)send:(NSString*)message
           to:(NSString*)receiver
    onSuccess:(void (^)())onSuccess
    onFailure:(void (^)(NSError*))onFailure {
  _nativeConferenceClient->Send(
      [message UTF8String], [receiver UTF8String],
      [=] {
        if (onSuccess != nil)
          onSuccess();
      },
      [=](std::unique_ptr<woogeen::conference::ConferenceException> e) {
        [self triggerOnFailure:onFailure withException:(std::move(e))];
      });
}

typedef void (^SuccessBlock)();
std::function<void()> PlayPauseSuccessCallback(SuccessBlock on_success) {
  return [=]() {
    if (on_success != nil)
      on_success();
  };
}

typedef void (^FailureBlock)(NSError*);
std::function<void(std::unique_ptr<woogeen::conference::ConferenceException>)>
PlayPauseFailureCallback(FailureBlock on_failure,
                         __weak RTCConferenceClient* client) {
  return [=](std::unique_ptr<woogeen::conference::ConferenceException> e) {
    [client triggerOnFailure:on_failure withException:(std::move(e))];
  };
}

- (void)playAudio:(RTCStream*)stream
        onSuccess:(void (^)())onSuccess
        onFailure:(void (^)(NSError*))onFailure {
  if ([stream isKindOfClass:[RTCLocalStream class]]) {
    std::shared_ptr<woogeen::base::LocalStream> nativeStream =
        [(RTCLocalStream*)stream nativeLocalStream];
    _nativeConferenceClient->PlayAudio(
        nativeStream, PlayPauseSuccessCallback(onSuccess),
        PlayPauseFailureCallback(onFailure, self));
  } else {
    std::shared_ptr<woogeen::base::RemoteStream> nativeStream =
        [(RTCRemoteStream*)stream nativeRemoteStream];
    _nativeConferenceClient->PlayAudio(
        nativeStream, PlayPauseSuccessCallback(onSuccess),
        PlayPauseFailureCallback(onFailure, self));
  }
}

- (void)pauseAudio:(RTCStream*)stream
         onSuccess:(void (^)())onSuccess
         onFailure:(void (^)(NSError*))onFailure {
  if ([stream isKindOfClass:[RTCLocalStream class]]) {
    std::shared_ptr<woogeen::base::LocalStream> nativeStream =
        [(RTCLocalStream*)stream nativeLocalStream];
    _nativeConferenceClient->PauseAudio(
        nativeStream, PlayPauseSuccessCallback(onSuccess),
        PlayPauseFailureCallback(onFailure, self));
  } else {
    std::shared_ptr<woogeen::base::RemoteStream> nativeStream =
        [(RTCRemoteStream*)stream nativeRemoteStream];
    _nativeConferenceClient->PauseAudio(
        nativeStream, PlayPauseSuccessCallback(onSuccess),
        PlayPauseFailureCallback(onFailure, self));
  }
}

- (void)playVideo:(RTCStream*)stream
        onSuccess:(void (^)())onSuccess
        onFailure:(void (^)(NSError*))onFailure {
  if ([stream isKindOfClass:[RTCLocalStream class]]) {
    std::shared_ptr<woogeen::base::LocalStream> nativeStream =
        [(RTCLocalStream*)stream nativeLocalStream];
    _nativeConferenceClient->PlayVideo(
        nativeStream, PlayPauseSuccessCallback(onSuccess),
        PlayPauseFailureCallback(onFailure, self));
  } else {
    std::shared_ptr<woogeen::base::RemoteStream> nativeStream =
        [(RTCRemoteStream*)stream nativeRemoteStream];
    _nativeConferenceClient->PlayVideo(
        nativeStream, PlayPauseSuccessCallback(onSuccess),
        PlayPauseFailureCallback(onFailure, self));
  }
}

- (void)pauseVideo:(RTCStream*)stream
         onSuccess:(void (^)())onSuccess
         onFailure:(void (^)(NSError*))onFailure {
  if ([stream isKindOfClass:[RTCLocalStream class]]) {
    std::shared_ptr<woogeen::base::LocalStream> nativeStream =
        [(RTCLocalStream*)stream nativeLocalStream];
    _nativeConferenceClient->PauseVideo(
        nativeStream, PlayPauseSuccessCallback(onSuccess),
        PlayPauseFailureCallback(onFailure, self));
  } else {
    std::shared_ptr<woogeen::base::RemoteStream> nativeStream =
        [(RTCRemoteStream*)stream nativeRemoteStream];
    _nativeConferenceClient->PauseVideo(
        nativeStream, PlayPauseSuccessCallback(onSuccess),
        PlayPauseFailureCallback(onFailure, self));
  }
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

- (void)getRegionOfStream:(RTCRemoteStream*)stream
            inMixedStream:(RTCRemoteMixedStream*)mixedStream
                onSuccess:(void (^)(NSString*))onSuccess
                onFailure:(nullable void (^)(NSError*))onFailure {
  auto nativeStreamRefPtr = [stream nativeStream];
  std::shared_ptr<woogeen::base::RemoteStream> nativeStream(
      std::static_pointer_cast<woogeen::base::RemoteStream>(
          nativeStreamRefPtr));
  std::shared_ptr<woogeen::conference::RemoteMixedStream> nativeMixedStream(
      std::static_pointer_cast<woogeen::conference::RemoteMixedStream>(
          [mixedStream nativeStream]));
  _nativeConferenceClient->GetRegion(
      nativeStream, nativeMixedStream,
      [=](std::string region_id) {
        if (onSuccess) {
          onSuccess([NSString stringWithUTF8String:region_id.c_str()]);
        }
      },
      [=](std::unique_ptr<woogeen::conference::ConferenceException> e) {
        [self triggerOnFailure:onFailure withException:(std::move(e))];
      });
}

- (void)setRegionOfStream:(RTCRemoteStream*)stream
               toRegionId:(NSString*)regionId
            inMixedStream:(RTCRemoteMixedStream*)mixedStream
                onSuccess:(nullable void (^)())onSuccess
                onFailure:(nullable void (^)(NSError*))onFailure {
  auto nativeStreamRefPtr = [stream nativeStream];
  std::shared_ptr<woogeen::base::RemoteStream> nativeStream(
      std::static_pointer_cast<woogeen::base::RemoteStream>(
          nativeStreamRefPtr));
  std::shared_ptr<woogeen::conference::RemoteMixedStream> nativeMixedStream(
      std::static_pointer_cast<woogeen::conference::RemoteMixedStream>(
          [mixedStream nativeStream]));
  auto nativeRegionid = [regionId UTF8String];
  _nativeConferenceClient->SetRegion(
      nativeStream, nativeMixedStream, nativeRegionid,
      [=]() {
        if (onSuccess) {
          onSuccess();
        }
      },
      [=](std::unique_ptr<woogeen::conference::ConferenceException> e) {
        [self triggerOnFailure:onFailure withException:(std::move(e))];
      });
}

- (void)getConnectionStatsForStream:(RTCStream*)stream
                          onSuccess:(void (^)(RTCConnectionStats*))onSuccess
                          onFailure:(void (^)(NSError*))onFailure {
  auto nativeStreamRefPtr = [stream nativeStream];
  _nativeConferenceClient->GetConnectionStats(
      nativeStreamRefPtr,
      [=](std::shared_ptr<woogeen::base::ConnectionStats> native_stats) {
        if (onSuccess) {
          onSuccess([[RTCConnectionStats alloc]
              initWithNativeStats:*native_stats]);
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

@implementation RTCConferenceClient (Internal)

- (RTCLocalStream*)publishedStreamWithId:(NSString*)streamId {
  return _publishedStreams[streamId];
}

@end
