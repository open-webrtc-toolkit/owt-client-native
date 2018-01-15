//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <string>
#import "talk/ics/sdk/base/objc/RTCConnectionStats+Internal.h"
#import "talk/ics/sdk/base/objc/RTCStream+Internal.h"
#import "talk/ics/sdk/base/objc/RTCLocalStream+Internal.h"
#import "talk/ics/sdk/base/objc/RTCRemoteStream+Internal.h"
#import "talk/ics/sdk/base/objc/RTCMediaCodec+Internal.h"
#import "talk/ics/sdk/include/objc/Woogeen/RTCErrors.h"
#import "talk/ics/sdk/include/objc/Woogeen/RTCConferenceClient.h"
#import "talk/ics/sdk/include/objc/Woogeen/RTCConferenceErrors.h"
#import "talk/ics/sdk/conference/conferencesocketsignalingchannel.h"
#import "talk/ics/sdk/conference/objc/ConferenceClientObserverObjcImpl.h"
#import "talk/ics/sdk/conference/objc/RTCConferenceClient+Internal.h"
#import "talk/ics/sdk/conference/objc/RTCConferenceSubscribeOptions+Internal.h"
#import "talk/ics/sdk/conference/objc/RTCConferenceUser+Internal.h"
#import "talk/ics/sdk/include/cpp/ics/conference/conferenceclient.h"
#import "talk/ics/sdk/include/cpp/ics/conference/remotemixedstream.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
#import "webrtc/sdk/objc/Framework/Classes/PeerConnection/RTCIceServer+Private.h"

@implementation RTCConferenceClient {
  std::shared_ptr<ics::conference::ConferenceClient> _nativeConferenceClient;
  std::vector<ics::conference::ConferenceClientObserverObjcImpl*> _observers;
  NSMutableDictionary<NSString*, RTCLocalStream*>* _publishedStreams;
}

- (instancetype)initWithConfiguration:
    (RTCConferenceClientConfiguration*)config {
  self = [super init];
  ics::conference::ConferenceClientConfiguration* nativeConfig =
      new ics::conference::ConferenceClientConfiguration();
  std::vector<ics::base::IceServer> iceServers;
  for (RTCIceServer* server in config.ICEServers) {
    ics::base::IceServer iceServer;
    iceServer.urls = server.nativeServer.urls;
    iceServer.username = server.nativeServer.username;
    iceServer.password = server.nativeServer.password;
    iceServers.push_back(iceServer);
  }
  nativeConfig->ice_servers = iceServers;
  /*
  nativeConfig->max_audio_bandwidth = [config maxAudioBandwidth];
  nativeConfig->max_video_bandwidth = [config maxVideoBandwidth];
  nativeConfig->audio_codec =
      [RTCMediaCodec nativeAudioCodec:config.audioCodec];
  nativeConfig->video_codec =
      [RTCMediaCodec nativeVideoCodec:config.videoCodec];
  nativeConfig->candidate_network_policy =
      ([config candidateNetworkPolicy] == RTCCandidateNetworkPolicyLowCost)
          ? ics::base::ClientConfiguration::CandidateNetworkPolicy::kLowCost
          : ics::base::ClientConfiguration::CandidateNetworkPolicy::kAll;
  _nativeConferenceClient =
      ics::conference::ConferenceClient::Create(*nativeConfig);
  _publishedStreams = [[NSMutableDictionary alloc] init];*/
  return self;
}

- (void)addObserver:(id<RTCConferenceClientObserver>)observer {
  auto ob =
      new ics::conference::ConferenceClientObserverObjcImpl(observer, self);
  _observers.push_back(ob);
  _nativeConferenceClient->AddObserver(*ob);
}

- (void)triggerOnFailure:(void (^)(NSError*))onFailure
           withException:
               (std::unique_ptr<ics::conference::ConferenceException>)e {
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
  auto it = std::find_if(
      _observers.begin(),
      _observers.end(),
      [=](const ics::conference::ConferenceClientObserverObjcImpl* observerObjcImpl) -> bool {
        return observerObjcImpl->ObjcObserver() == observer;
      });
  if (it == _observers.end()) {
    LOG(LS_WARNING) << "Cannot remove a non-existing element.";
    return;
  }
  auto o = *it;
  _nativeConferenceClient->RemoveObserver(*o);
  _observers.erase(it);
  delete o;
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
  /*
  _nativeConferenceClient->Join(
      nativeToken,
      [=](std::shared_ptr<ics::conference::Participant> user) {
        RTCConferenceUser* conferenceUser =
            [[RTCConferenceUser alloc] initWithNativeUser:user];
        if (onSuccess != nil)
          onSuccess(conferenceUser);
      },
      [=](std::unique_ptr<ics::conference::ConferenceException> e) {
        [self triggerOnFailure:onFailure withException:(std::move(e))];
      });*/
}

- (void)publish:(RTCLocalStream*)stream
      onSuccess:(void (^)())onSuccess
      onFailure:(void (^)(NSError*))onFailure {
  /*
  auto nativeStreamRefPtr = [stream nativeStream];
  std::shared_ptr<ics::base::LocalStream> nativeStream(
      std::static_pointer_cast<ics::base::LocalStream>(nativeStreamRefPtr));
  _nativeConferenceClient->Publish(
      nativeStream,
      [=]() {
        [_publishedStreams setObject:stream forKey:[stream streamId]];
        if (onSuccess != nil)
          onSuccess();
      },
      [=](std::unique_ptr<ics::conference::ConferenceException> e) {
        [self triggerOnFailure:onFailure withException:(std::move(e))];
      });*/
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
  /*
  if (options == nil) {
    options = [[RTCConferenceSubscribeOptions alloc] init];
  }
  auto nativeStreamRefPtr = [stream nativeStream];
  std::shared_ptr<ics::base::RemoteStream> nativeStream(
      std::static_pointer_cast<ics::base::RemoteStream>(nativeStreamRefPtr));
  _nativeConferenceClient->Subscribe(
      nativeStream, [options nativeSubscribeOptions],
      [=](std::shared_ptr<ics::base::RemoteStream> stream) {
        RTCRemoteStream* remote_stream =
            [[RTCRemoteStream alloc] initWithNativeStream:stream];
        onSuccess(remote_stream);
      },
      [=](std::unique_ptr<ics::conference::ConferenceException> e) {
        [self triggerOnFailure:onFailure withException:(std::move(e))];
      });*/
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
      [=](std::unique_ptr<ics::conference::ConferenceException> e) {
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
      [=](std::unique_ptr<ics::conference::ConferenceException> e) {
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
std::function<void(std::unique_ptr<ics::conference::ConferenceException>)>
PlayPauseFailureCallback(FailureBlock on_failure,
                         __weak RTCConferenceClient* client) {
  return [=](std::unique_ptr<ics::conference::ConferenceException> e) {
    [client triggerOnFailure:on_failure withException:(std::move(e))];
  };
}

- (void)leaveWithOnSuccess:(void (^)())onSuccess
                 onFailure:(void (^)(NSError*))onFailure {
  _nativeConferenceClient->Leave(
      [=]() {
        if (onSuccess != nil)
          onSuccess();
      },
      [=](std::unique_ptr<ics::conference::ConferenceException> e) {
        [self triggerOnFailure:onFailure withException:(std::move(e))];
      });
}

- (void)getConnectionStatsForStream:(RTCStream*)stream
                          onSuccess:(void (^)(RTCConnectionStats*))onSuccess
                          onFailure:(void (^)(NSError*))onFailure {
  /*
  auto nativeStreamRefPtr = [stream nativeStream];
  _nativeConferenceClient->GetConnectionStats(
      nativeStreamRefPtr,
      [=](std::shared_ptr<ics::base::ConnectionStats> native_stats) {
        if (onSuccess) {
          onSuccess([[RTCConnectionStats alloc]
              initWithNativeStats:*native_stats]);
        }
      },
      [=](std::unique_ptr<ics::conference::ConferenceException> e) {
        [self triggerOnFailure:onFailure withException:(std::move(e))];
      });*/
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
