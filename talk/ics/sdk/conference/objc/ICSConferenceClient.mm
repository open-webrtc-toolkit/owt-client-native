//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#include <string>
#include <functional>

#import <Foundation/Foundation.h>
#import "talk/ics/sdk/base/objc/ICSConnectionStats+Internal.h"
#import "talk/ics/sdk/base/objc/ICSStream+Private.h"
#import "talk/ics/sdk/base/objc/ICSLocalStream+Private.h"
#import "talk/ics/sdk/base/objc/ICSRemoteStream+Private.h"
#import "talk/ics/sdk/base/objc/ICSMediaFormat+Private.h"
#import "talk/ics/sdk/include/objc/ICS/ICSErrors.h"
#import "talk/ics/sdk/include/objc/ICS/ICSConferenceClient.h"
#import "talk/ics/sdk/include/objc/ICS/ICSConferenceErrors.h"
#import "talk/ics/sdk/include/objc/ICS/ICSConferenceInfo.h"
#import "talk/ics/sdk/conference/conferencesocketsignalingchannel.h"
#import "talk/ics/sdk/conference/objc/ConferenceClientObserverObjcImpl.h"
#import "talk/ics/sdk/conference/objc/ICSConferenceClient+Internal.h"
#import "talk/ics/sdk/conference/objc/ICSConferenceInfo+Private.h"
#import "talk/ics/sdk/conference/objc/ICSConferenceSubscription+Private.h"
#import "talk/ics/sdk/conference/objc/ICSConferenceParticipant+Private.h"
#import "talk/ics/sdk/conference/objc/ICSConferencePublication+Private.h"
#import "talk/ics/sdk/include/cpp/ics/conference/conferenceclient.h"
#import "talk/ics/sdk/include/cpp/ics/conference/remotemixedstream.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
#import "webrtc/sdk/objc/Framework/Classes/PeerConnection/RTCIceServer+Private.h"

@implementation ICSConferenceClient {
  std::shared_ptr<ics::conference::ConferenceClient> _nativeConferenceClient;
  std::unique_ptr<
      ics::conference::ConferenceClientObserverObjcImpl,
      std::function<void(ics::conference::ConferenceClientObserverObjcImpl*)>>
      _observer;
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
          : ics::base::ClientConfiguration::CandidateNetworkPolicy::kAll;*/
  _nativeConferenceClient =
      ics::conference::ConferenceClient::Create(*nativeConfig);
  _publishedStreams = [[NSMutableDictionary alloc] init];
  return self;
}

- (void)triggerOnFailure:(void (^)(NSError*))onFailure
           withException:
               (std::unique_ptr<ics::base::Exception>)e {
  if (onFailure == nil)
    return;
  NSError* err = [[NSError alloc]
      initWithDomain:ICSErrorDomain
                code:ICSConferenceErrorUnknown
            userInfo:[[NSDictionary alloc]
                         initWithObjectsAndKeys:
                             [NSString stringForStdString:e->Message()],
                             NSLocalizedDescriptionKey, nil]];
  onFailure(err);
}

- (void)joinWithToken:(NSString*)token
            onSuccess:(void (^)(ICSConferenceInfo*))onSuccess
            onFailure:(void (^)(NSError*))onFailure {
  if (token == nil) {
    if (onFailure != nil) {
      NSError* err = [[NSError alloc]
          initWithDomain:ICSErrorDomain
                    code:ICSConferenceErrorUnknown
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
      [=](std::shared_ptr<ics::conference::ConferenceInfo> info) {
        if (onSuccess != nil)
          onSuccess([[ICSConferenceInfo alloc]
              initWithNativeInfo:info]);
      },
      [=](std::unique_ptr<ics::base::Exception> e) {
        [self triggerOnFailure:onFailure withException:(std::move(e))];
      });
}

- (void)publish:(ICSLocalStream*)stream
      onSuccess:(void (^)(ICSConferencePublication*))onSuccess
      onFailure:(void (^)(NSError*))onFailure {
  auto nativeStreamRefPtr = [stream nativeStream];
  std::shared_ptr<ics::base::LocalStream> nativeStream(
      std::static_pointer_cast<ics::base::LocalStream>(nativeStreamRefPtr));
  _nativeConferenceClient->Publish(
      nativeStream,
      [=](std::shared_ptr<ics::conference::ConferencePublication> publication) {
        [_publishedStreams setObject:stream forKey:[stream streamId]];
        if (onSuccess != nil)
          onSuccess([[ICSConferencePublication alloc]
              initWithNativePublication:publication]);
      },
      [=](std::unique_ptr<ics::base::Exception> e) {
        [self triggerOnFailure:onFailure withException:(std::move(e))];
      });
}

- (void)subscribe:(ICSRemoteStream*)stream
        onSuccess:(void (^)(ICSConferenceSubscription*))onSuccess
        onFailure:(void (^)(NSError*))onFailure {
  ICSConferenceSubscriptionOptions* options =
      [[ICSConferenceSubscriptionOptions alloc] init];
  [self subscribe:stream
      withOptions:options
        onSuccess:onSuccess
        onFailure:onFailure];
}

- (void)subscribe:(ICSRemoteStream*)stream
      withOptions:(ICSConferenceSubscriptionOptions*)options
        onSuccess:(void (^)(ICSConferenceSubscription*))onSuccess
        onFailure:(void (^)(NSError*))onFailure {
  if (options == nil) {
    options = [[ICSConferenceSubscriptionOptions alloc] init];
  }
  auto nativeStreamRefPtr = [stream nativeStream];
  std::shared_ptr<ics::base::RemoteStream> nativeStream(
      std::static_pointer_cast<ics::base::RemoteStream>(nativeStreamRefPtr));
  _nativeConferenceClient->Subscribe(
      nativeStream, [options nativeSubscriptionOptions],
      [=](std::shared_ptr<ics::conference::ConferenceSubscription>
              subscription) {
        ICSConferenceSubscription* sub = [[ICSConferenceSubscription alloc]
            initWithNativeSubscription:subscription];
        onSuccess(sub);
      },
      [=](std::unique_ptr<ics::base::Exception> e) {
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
      [=](std::unique_ptr<ics::base::Exception> e) {
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
      [=](std::unique_ptr<ics::base::Exception> e) {
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
std::function<void(std::unique_ptr<ics::base::Exception>)>
PlayPauseFailureCallback(FailureBlock on_failure,
                         __weak ICSConferenceClient* client) {
  return [=](std::unique_ptr<ics::base::Exception> e) {
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
      [=](std::unique_ptr<ics::base::Exception> e) {
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
      [=](std::unique_ptr<ics::base::Exception> e) {
        [self triggerOnFailure:onFailure withException:(std::move(e))];
      });*/
}

- (void)setDelegate:(id<ICSConferenceClientDelegate>)delegate {
  _observer = std::unique_ptr<
      ics::conference::ConferenceClientObserverObjcImpl,
      std::function<void(ics::conference::ConferenceClientObserverObjcImpl*)>>(
      new ics::conference::ConferenceClientObserverObjcImpl(self, delegate),
      [&self](ics::conference::ConferenceClientObserverObjcImpl* observer) {
        self->_nativeConferenceClient->RemoveObserver(*observer);
      });
  _nativeConferenceClient->AddObserver(*_observer.get());
  _delegate = delegate;
}

@end

@implementation ICSConferenceClient (Internal)

- (ICSLocalStream*)publishedStreamWithId:(NSString*)streamId {
  return _publishedStreams[streamId];
}

@end
