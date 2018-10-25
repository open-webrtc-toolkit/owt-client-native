//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#include <string>
#include <functional>

#import <Foundation/Foundation.h>
#import "talk/oms/sdk/base/objc/OMSStream+Private.h"
#import "talk/oms/sdk/base/objc/OMSLocalStream+Private.h"
#import "talk/oms/sdk/base/objc/OMSPublishOptions+Private.h"
#import "talk/oms/sdk/base/objc/OMSRemoteStream+Private.h"
#import "talk/oms/sdk/base/objc/OMSMediaFormat+Private.h"
#import "talk/oms/sdk/include/objc/OMS/OMSErrors.h"
#import "talk/oms/sdk/include/objc/OMS/OMSConferenceClient.h"
#import "talk/oms/sdk/include/objc/OMS/OMSConferenceErrors.h"
#import "talk/oms/sdk/include/objc/OMS/OMSConferenceInfo.h"
#import "talk/oms/sdk/conference/conferencesocketsignalingchannel.h"
#import "talk/oms/sdk/conference/objc/ConferenceClientObserverObjcImpl.h"
#import "talk/oms/sdk/conference/objc/OMSConferenceClient+Internal.h"
#import "talk/oms/sdk/conference/objc/OMSConferenceInfo+Private.h"
#import "talk/oms/sdk/conference/objc/OMSConferenceSubscription+Private.h"
#import "talk/oms/sdk/conference/objc/OMSConferenceParticipant+Private.h"
#import "talk/oms/sdk/conference/objc/OMSConferencePublication+Private.h"
#import "talk/oms/sdk/include/cpp/oms/conference/conferenceclient.h"
#import "talk/oms/sdk/include/cpp/oms/conference/remotemixedstream.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
#import "webrtc/sdk/objc/Framework/Classes/PeerConnection/RTCIceServer+Private.h"

@implementation OMSConferenceClient {
  std::shared_ptr<oms::conference::ConferenceClient> _nativeConferenceClient;
  std::unique_ptr<
      oms::conference::ConferenceClientObserverObjcImpl,
      std::function<void(oms::conference::ConferenceClientObserverObjcImpl*)>>
      _observer;
  NSMutableDictionary<NSString*, OMSLocalStream*>* _publishedStreams;
}

- (instancetype)initWithConfiguration:
    (OMSConferenceClientConfiguration*)config {
  self = [super init];
  oms::conference::ConferenceClientConfiguration* nativeConfig =
      new oms::conference::ConferenceClientConfiguration();
  std::vector<oms::base::IceServer> iceServers;
  for (RTCIceServer* server in config.rtcConfiguration.iceServers) {
    oms::base::IceServer iceServer;
    iceServer.urls = server.nativeServer.urls;
    iceServer.username = server.nativeServer.username;
    iceServer.password = server.nativeServer.password;
    iceServers.push_back(iceServer);
  }
  nativeConfig->ice_servers = iceServers;
  nativeConfig->candidate_network_policy =
      config.rtcConfiguration.candidateNetworkPolicy ==
              RTCCandidateNetworkPolicyLowCost
          ? oms::base::ClientConfiguration::CandidateNetworkPolicy::kLowCost
          : oms::base::ClientConfiguration::CandidateNetworkPolicy::kAll;
  _nativeConferenceClient =
      oms::conference::ConferenceClient::Create(*nativeConfig);
  _publishedStreams = [[NSMutableDictionary alloc] init];
  return self;
}

- (void)triggerOnFailure:(void (^)(NSError*))onFailure
           withException:
               (std::unique_ptr<oms::base::Exception>)e {
  if (onFailure == nil)
    return;
  NSError* err = [[NSError alloc]
      initWithDomain:OMSErrorDomain
                code:OMSConferenceErrorUnknown
            userInfo:[[NSDictionary alloc]
                         initWithObjectsAndKeys:
                             [NSString stringForStdString:e->Message()],
                             NSLocalizedDescriptionKey, nil]];
  onFailure(err);
}

- (void)joinWithToken:(NSString*)token
            onSuccess:(void (^)(OMSConferenceInfo*))onSuccess
            onFailure:(void (^)(NSError*))onFailure {
  if (token == nil) {
    if (onFailure != nil) {
      NSError* err = [[NSError alloc]
          initWithDomain:OMSErrorDomain
                    code:OMSConferenceErrorUnknown
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
      [=](std::shared_ptr<oms::conference::ConferenceInfo> info) {
        if (onSuccess != nil)
          onSuccess([[OMSConferenceInfo alloc]
              initWithNativeInfo:info]);
      },
      [=](std::unique_ptr<oms::base::Exception> e) {
        [self triggerOnFailure:onFailure withException:(std::move(e))];
      });
}

- (void)publish:(OMSLocalStream*)stream
    withOptions:(OMSPublishOptions*)options
      onSuccess:(void (^)(OMSConferencePublication*))onSuccess
      onFailure:(void (^)(NSError*))onFailure {
  RTC_CHECK(stream);
  auto nativeStreamRefPtr = [stream nativeStream];
  std::shared_ptr<oms::base::LocalStream> nativeStream(
      std::static_pointer_cast<oms::base::LocalStream>(nativeStreamRefPtr));
  if (options == nil) {
    _nativeConferenceClient->Publish(
        nativeStream,
        [=](std::shared_ptr<oms::conference::ConferencePublication>
                publication) {
          [_publishedStreams setObject:stream forKey:[stream streamId]];
          if (onSuccess != nil)
            onSuccess([[OMSConferencePublication alloc]
                initWithNativePublication:publication]);
        },
        [=](std::unique_ptr<oms::base::Exception> e) {
          [self triggerOnFailure:onFailure withException:(std::move(e))];
        });
  } else {
    _nativeConferenceClient->Publish(
        nativeStream, *[options nativePublishOptions].get(),
        [=](std::shared_ptr<oms::conference::ConferencePublication>
                publication) {
          [_publishedStreams setObject:stream forKey:[stream streamId]];
          if (onSuccess != nil)
            onSuccess([[OMSConferencePublication alloc]
                initWithNativePublication:publication]);
        },
        [=](std::unique_ptr<oms::base::Exception> e) {
          [self triggerOnFailure:onFailure withException:(std::move(e))];
        });
  }
}

- (void)subscribe:(OMSRemoteStream*)stream
        onSuccess:(void (^)(OMSConferenceSubscription*))onSuccess
        onFailure:(void (^)(NSError*))onFailure {
  OMSConferenceSubscribeOptions* options =
      [[OMSConferenceSubscribeOptions alloc] init];
  [self subscribe:stream
      withOptions:options
        onSuccess:onSuccess
        onFailure:onFailure];
}

- (void)subscribe:(OMSRemoteStream*)stream
      withOptions:(OMSConferenceSubscribeOptions*)options
        onSuccess:(void (^)(OMSConferenceSubscription*))onSuccess
        onFailure:(void (^)(NSError*))onFailure {
  RTC_CHECK(stream);
  auto nativeStreamRefPtr = [stream nativeStream];
  std::shared_ptr<oms::base::RemoteStream> nativeStream(
      std::static_pointer_cast<oms::base::RemoteStream>(nativeStreamRefPtr));
  if (options == nil) {
    _nativeConferenceClient->Subscribe(
        nativeStream,
        [=](std::shared_ptr<oms::conference::ConferenceSubscription>
                subscription) {
          OMSConferenceSubscription* sub = [[OMSConferenceSubscription alloc]
              initWithNativeSubscription:subscription];
          if (onSuccess != nil) {
            onSuccess(sub);
          }
        },
        [=](std::unique_ptr<oms::base::Exception> e) {
          [self triggerOnFailure:onFailure withException:(std::move(e))];
        });
  } else {
    _nativeConferenceClient->Subscribe(
        nativeStream, *[options nativeSubscribeOptions].get(),
        [=](std::shared_ptr<oms::conference::ConferenceSubscription>
                subscription) {
          OMSConferenceSubscription* sub = [[OMSConferenceSubscription alloc]
              initWithNativeSubscription:subscription];
          if (onSuccess != nil) {
            onSuccess(sub);
          }
        },
        [=](std::unique_ptr<oms::base::Exception> e) {
          [self triggerOnFailure:onFailure withException:(std::move(e))];
        });
  }
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
      [=](std::unique_ptr<oms::base::Exception> e) {
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
      [=](std::unique_ptr<oms::base::Exception> e) {
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
std::function<void(std::unique_ptr<oms::base::Exception>)>
PlayPauseFailureCallback(FailureBlock on_failure,
                         __weak OMSConferenceClient* client) {
  return [=](std::unique_ptr<oms::base::Exception> e) {
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
      [=](std::unique_ptr<oms::base::Exception> e) {
        [self triggerOnFailure:onFailure withException:(std::move(e))];
      });
}

- (void)setDelegate:(id<OMSConferenceClientDelegate>)delegate {
  _observer = std::unique_ptr<
      oms::conference::ConferenceClientObserverObjcImpl,
      std::function<void(oms::conference::ConferenceClientObserverObjcImpl*)>>(
      new oms::conference::ConferenceClientObserverObjcImpl(self, delegate),
      [&self](oms::conference::ConferenceClientObserverObjcImpl* observer) {
        self->_nativeConferenceClient->RemoveObserver(*observer);
      });
  _nativeConferenceClient->AddObserver(*_observer.get());
  _delegate = delegate;
}

@end

@implementation OMSConferenceClient (Internal)

- (OMSLocalStream*)publishedStreamWithId:(NSString*)streamId {
  return _publishedStreams[streamId];
}

@end
