//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <string>
#import "talk/ics/sdk/base/objc/ICSConnectionStats+Internal.h"
#import "talk/ics/sdk/base/objc/ICSStream+Internal.h"
#import "talk/ics/sdk/base/objc/ICSLocalStream+Internal.h"
#import "talk/ics/sdk/base/objc/ICSRemoteStream+Internal.h"
#import "talk/ics/sdk/base/objc/ICSMediaCodec+Internal.h"
#import "talk/ics/sdk/include/objc/ICS/ICSErrors.h"
#import "talk/ics/sdk/include/objc/ICS/ICSConferenceClient.h"
#import "talk/ics/sdk/include/objc/ICS/ICSConferenceErrors.h"
#import "talk/ics/sdk/conference/conferencesocketsignalingchannel.h"
#import "talk/ics/sdk/conference/objc/ConferenceClientObserverObjcImpl.h"
#import "talk/ics/sdk/conference/objc/ICSConferenceClient+Internal.h"
#import "talk/ics/sdk/conference/objc/ICSConferenceSubscribeOptions+Internal.h"
#import "talk/ics/sdk/conference/objc/ICSConferenceUser+Internal.h"
#import "talk/ics/sdk/include/cpp/ics/conference/conferenceclient.h"
#import "talk/ics/sdk/include/cpp/ics/conference/remotemixedstream.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
#import "webrtc/sdk/objc/Framework/Classes/PeerConnection/RTCIceServer+Private.h"

@implementation ICSConferenceClient {
  std::shared_ptr<ics::conference::ConferenceClient> _nativeConferenceClient;
  std::vector<ics::conference::ConferenceClientObserverObjcImpl*> _observers;
  NSMutableDictionary<NSString*, ICSLocalStream*>* _publishedStreams;
}

- (instancetype)initWithConfiguration:
    (ICSConferenceClientConfiguration*)config {
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

- (void)addObserver:(id<ICSConferenceClientObserver>)observer {
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

- (void)removeObserver:(id<ICSConferenceClientObserver>)observer {
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
            onSuccess:(void (^)(ICSConferenceUser*))onSuccess
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
        ICSConferenceUser* conferenceUser =
            [[ICSConferenceUser alloc] initWithNativeUser:user];
        if (onSuccess != nil)
          onSuccess(conferenceUser);
      },
      [=](std::unique_ptr<ics::conference::ConferenceException> e) {
        [self triggerOnFailure:onFailure withException:(std::move(e))];
      });*/
}

- (void)publish:(ICSLocalStream*)stream
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

- (void)subscribe:(ICSRemoteStream*)stream
        onSuccess:(void (^)(ICSRemoteStream*))onSuccess
        onFailure:(void (^)(NSError*))onFailure {
  ICSConferenceSubscribeOptions* options =
      [[ICSConferenceSubscribeOptions alloc] init];
  [self subscribe:stream
      withOptions:options
        onSuccess:onSuccess
        onFailure:onFailure];
}

- (void)subscribe:(ICSRemoteStream*)stream
      withOptions:(ICSConferenceSubscribeOptions*)options
        onSuccess:(void (^)(ICSRemoteStream*))onSuccess
        onFailure:(void (^)(NSError*))onFailure {
  /*
  if (options == nil) {
    options = [[ICSConferenceSubscribeOptions alloc] init];
  }
  auto nativeStreamRefPtr = [stream nativeStream];
  std::shared_ptr<ics::base::RemoteStream> nativeStream(
      std::static_pointer_cast<ics::base::RemoteStream>(nativeStreamRefPtr));
  _nativeConferenceClient->Subscribe(
      nativeStream, [options nativeSubscribeOptions],
      [=](std::shared_ptr<ics::base::RemoteStream> stream) {
        ICSRemoteStream* remote_stream =
            [[ICSRemoteStream alloc] initWithNativeStream:stream];
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
                         __weak ICSConferenceClient* client) {
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

- (void)getConnectionStatsForStream:(ICSStream*)stream
                          onSuccess:(void (^)(ICSConnectionStats*))onSuccess
                          onFailure:(void (^)(NSError*))onFailure {
  /*
  auto nativeStreamRefPtr = [stream nativeStream];
  _nativeConferenceClient->GetConnectionStats(
      nativeStreamRefPtr,
      [=](std::shared_ptr<ics::base::ConnectionStats> native_stats) {
        if (onSuccess) {
          onSuccess([[ICSConnectionStats alloc]
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

@implementation ICSConferenceClient (Internal)

- (ICSLocalStream*)publishedStreamWithId:(NSString*)streamId {
  return _publishedStreams[streamId];
}

@end
