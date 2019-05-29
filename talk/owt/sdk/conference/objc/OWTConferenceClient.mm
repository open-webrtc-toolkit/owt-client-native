// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include <string>
#include <functional>
#import <Foundation/Foundation.h>
#import "talk/owt/sdk/base/objc/OWTStream+Private.h"
#import "talk/owt/sdk/base/objc/OWTLocalStream+Private.h"
#import "talk/owt/sdk/base/objc/OWTPublishOptions+Private.h"
#import "talk/owt/sdk/base/objc/OWTRemoteStream+Private.h"
#import "talk/owt/sdk/base/objc/OWTMediaFormat+Private.h"
#import "talk/owt/sdk/include/objc/OWT/OWTErrors.h"
#import "talk/owt/sdk/include/objc/OWT/OWTConferenceClient.h"
#import "talk/owt/sdk/include/objc/OWT/OWTConferenceErrors.h"
#import "talk/owt/sdk/include/objc/OWT/OWTConferenceInfo.h"
#import "talk/owt/sdk/conference/conferencesocketsignalingchannel.h"
#import "talk/owt/sdk/conference/objc/ConferenceClientObserverObjcImpl.h"
#import "talk/owt/sdk/conference/objc/OWTConferenceClient+Internal.h"
#import "talk/owt/sdk/conference/objc/OWTConferenceInfo+Private.h"
#import "talk/owt/sdk/conference/objc/OWTConferenceSubscription+Private.h"
#import "talk/owt/sdk/conference/objc/OWTConferenceParticipant+Private.h"
#import "talk/owt/sdk/conference/objc/OWTConferencePublication+Private.h"
#import "talk/owt/sdk/include/cpp/owt/conference/conferenceclient.h"
#import "talk/owt/sdk/include/cpp/owt/conference/remotemixedstream.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
#import "webrtc/sdk/objc/Framework/Classes/PeerConnection/RTCIceServer+Private.h"
@implementation OWTConferenceClient {
  std::shared_ptr<owt::conference::ConferenceClient> _nativeConferenceClient;
  std::unique_ptr<
      owt::conference::ConferenceClientObserverObjcImpl,
      std::function<void(owt::conference::ConferenceClientObserverObjcImpl*)>>
      _observer;
  NSMutableDictionary<NSString*, OWTLocalStream*>* _publishedStreams;
}
- (instancetype)initWithConfiguration:
    (OWTConferenceClientConfiguration*)config {
  self = [super init];
  owt::conference::ConferenceClientConfiguration* nativeConfig =
      new owt::conference::ConferenceClientConfiguration();
  std::vector<owt::base::IceServer> iceServers;
  for (RTCIceServer* server in config.rtcConfiguration.iceServers) {
    owt::base::IceServer iceServer;
    iceServer.urls = server.nativeServer.urls;
    iceServer.username = server.nativeServer.username;
    iceServer.password = server.nativeServer.password;
    iceServers.push_back(iceServer);
  }
  nativeConfig->ice_servers = iceServers;
  nativeConfig->candidate_network_policy =
      config.rtcConfiguration.candidateNetworkPolicy ==
              RTCCandidateNetworkPolicyLowCost
          ? owt::base::ClientConfiguration::CandidateNetworkPolicy::kLowCost
          : owt::base::ClientConfiguration::CandidateNetworkPolicy::kAll;
  _nativeConferenceClient =
      owt::conference::ConferenceClient::Create(*nativeConfig);
  _publishedStreams = [[NSMutableDictionary alloc] init];
  return self;
}
- (void)triggerOnFailure:(void (^)(NSError*))onFailure
           withException:
               (std::unique_ptr<owt::base::Exception>)e {
  if (onFailure == nil)
    return;
  NSError* err = [[NSError alloc]
      initWithDomain:OWTErrorDomain
                code:OWTConferenceErrorUnknown
            userInfo:[[NSDictionary alloc]
                         initWithObjectsAndKeys:
                             [NSString stringForStdString:e->Message()],
                             NSLocalizedDescriptionKey, nil]];
  onFailure(err);
}
- (void)joinWithToken:(NSString*)token
            onSuccess:(void (^)(OWTConferenceInfo*))onSuccess
            onFailure:(void (^)(NSError*))onFailure {
  if (token == nil) {
    if (onFailure != nil) {
      NSError* err = [[NSError alloc]
          initWithDomain:OWTErrorDomain
                    code:OWTConferenceErrorUnknown
                userInfo:[[NSDictionary alloc]
                             initWithObjectsAndKeys:@"Token cannot be nil.",
                                                    NSLocalizedDescriptionKey,
                                                    nil]];
      onFailure(err);
    }
    return;
  }
  const std::string nativeToken = [token UTF8String];
  __weak OWTConferenceClient *weakSelf = self;
  _nativeConferenceClient->Join(
      nativeToken,
      [=](std::shared_ptr<owt::conference::ConferenceInfo> info) {
        if (onSuccess != nil)
          onSuccess([[OWTConferenceInfo alloc]
              initWithNativeInfo:info]);
      },
      [=](std::unique_ptr<owt::base::Exception> e) {
        [weakSelf triggerOnFailure:onFailure withException:(std::move(e))];
      });
}
- (void)publish:(OWTLocalStream*)stream
    withOptions:(OWTPublishOptions*)options
      onSuccess:(void (^)(OWTConferencePublication*))onSuccess
      onFailure:(void (^)(NSError*))onFailure {
  RTC_CHECK(stream);
  auto nativeStreamRefPtr = [stream nativeStream];
  std::shared_ptr<owt::base::LocalStream> nativeStream(
      std::static_pointer_cast<owt::base::LocalStream>(nativeStreamRefPtr));
  if (options == nil) {
    _nativeConferenceClient->Publish(
        nativeStream,
        [=](std::shared_ptr<owt::conference::ConferencePublication>
                publication) {
          [_publishedStreams setObject:stream forKey:[stream streamId]];
          if (onSuccess != nil)
            onSuccess([[OWTConferencePublication alloc]
                initWithNativePublication:publication]);
        },
        [=](std::unique_ptr<owt::base::Exception> e) {
          [self triggerOnFailure:onFailure withException:(std::move(e))];
        });
  } else {
    _nativeConferenceClient->Publish(
        nativeStream, *[options nativePublishOptions].get(),
        [=](std::shared_ptr<owt::conference::ConferencePublication>
                publication) {
          [_publishedStreams setObject:stream forKey:[stream streamId]];
          if (onSuccess != nil)
            onSuccess([[OWTConferencePublication alloc]
                initWithNativePublication:publication]);
        },
        [=](std::unique_ptr<owt::base::Exception> e) {
          [self triggerOnFailure:onFailure withException:(std::move(e))];
        });
  }
}
- (void)subscribe:(OWTRemoteStream*)stream
        onSuccess:(void (^)(OWTConferenceSubscription*))onSuccess
        onFailure:(void (^)(NSError*))onFailure {
  OWTConferenceSubscribeOptions* options =
      [[OWTConferenceSubscribeOptions alloc] init];
  [self subscribe:stream
      withOptions:options
        onSuccess:onSuccess
        onFailure:onFailure];
}
- (void)subscribe:(OWTRemoteStream*)stream
      withOptions:(OWTConferenceSubscribeOptions*)options
        onSuccess:(void (^)(OWTConferenceSubscription*))onSuccess
        onFailure:(void (^)(NSError*))onFailure {
  RTC_CHECK(stream);
  auto nativeStreamRefPtr = [stream nativeStream];
  std::shared_ptr<owt::base::RemoteStream> nativeStream(
      std::static_pointer_cast<owt::base::RemoteStream>(nativeStreamRefPtr));
  if (options == nil) {
    _nativeConferenceClient->Subscribe(
        nativeStream,
        [=](std::shared_ptr<owt::conference::ConferenceSubscription>
                subscription) {
          OWTConferenceSubscription* sub = [[OWTConferenceSubscription alloc]
              initWithNativeSubscription:subscription];
          [stream setNativeStream:nativeStream];
          if (onSuccess != nil) {
            onSuccess(sub);
          }
        },
        [=](std::unique_ptr<owt::base::Exception> e) {
          [self triggerOnFailure:onFailure withException:(std::move(e))];
        });
  } else {
    _nativeConferenceClient->Subscribe(
        nativeStream, *[options nativeSubscribeOptions].get(),
        [=](std::shared_ptr<owt::conference::ConferenceSubscription>
                subscription) {
          OWTConferenceSubscription* sub = [[OWTConferenceSubscription alloc]
              initWithNativeSubscription:subscription];
          [stream setNativeStream:nativeStream];
          if (onSuccess != nil) {
            onSuccess(sub);
          }
        },
        [=](std::unique_ptr<owt::base::Exception> e) {
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
      [=](std::unique_ptr<owt::base::Exception> e) {
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
      [=](std::unique_ptr<owt::base::Exception> e) {
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
std::function<void(std::unique_ptr<owt::base::Exception>)>
PlayPauseFailureCallback(FailureBlock on_failure,
                         __weak OWTConferenceClient* client) {
  return [=](std::unique_ptr<owt::base::Exception> e) {
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
      [=](std::unique_ptr<owt::base::Exception> e) {
        [self triggerOnFailure:onFailure withException:(std::move(e))];
      });
}
- (void)setDelegate:(id<OWTConferenceClientDelegate>)delegate {
  if (delegate != nil) {
    __weak OWTConferenceClient *weakSelf = self;
    _observer = std::unique_ptr<
            owt::conference::ConferenceClientObserverObjcImpl,
            std::function<void(owt::conference::ConferenceClientObserverObjcImpl*)>>(
                    new owt::conference::ConferenceClientObserverObjcImpl(self, delegate),
                    [=](owt::conference::ConferenceClientObserverObjcImpl* observer) {
                        __strong OWTConferenceClient *strongSelf = weakSelf;
                        if (strongSelf != nil) {
                          strongSelf->_nativeConferenceClient->RemoveObserver(*observer);
                        }
                        delete observer;
                    });
    _nativeConferenceClient->AddObserver(*_observer.get());
  } else {
    _observer.reset();
  }
  _delegate = delegate;
}
@end
@implementation OWTConferenceClient (Internal)
- (OWTLocalStream*)publishedStreamWithId:(NSString*)streamId {
  return _publishedStreams[streamId];
}
@end
