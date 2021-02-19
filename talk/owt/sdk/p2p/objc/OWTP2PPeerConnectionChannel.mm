// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "Foundation/Foundation.h"
#import "talk/owt/sdk/include/objc/OWT/OWTErrors.h"
#import "talk/owt/sdk/include/objc/OWT/OWTP2PPeerConnectionChannelObserver.h"
#import "talk/owt/sdk/include/objc/OWT/OWTP2PErrors.h"
#import "talk/owt/sdk/p2p/objc/OWTP2PPeerConnectionChannel.h"
#import "talk/owt/sdk/p2p/objc/P2PPeerConnectionChannelObserverObjcImpl.h"
#import "talk/owt/sdk/p2p/objc/OWTP2PSignalingSenderObjcImpl.h"
#import "talk/owt/sdk/p2p/objc/OWTP2PPublication+Private.h"
#import "talk/owt/sdk/base/objc/OWTLocalStream+Private.h"
#import "talk/owt/sdk/base/objc/OWTMediaFormat+Private.h"
#import "talk/owt/sdk/base/objc/OWTStream+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
#import "webrtc/sdk/objc/api/peerconnection/RTCIceServer+Private.h"
#import "webrtc/sdk/objc/api/peerconnection/RTCLegacyStatsReport+Private.h"
#import "RTCLogging.h"
#include "talk/owt/sdk/p2p/p2ppeerconnectionchannel.h"
@implementation OWTP2PPeerConnectionChannel {
  owt::p2p::P2PPeerConnectionChannel* _nativeChannel;
  NSString* _remoteId;
}
- (instancetype)initWithConfiguration:(OWTP2PClientConfiguration*)config
                              localId:(NSString*)localId
                             remoteId:(NSString*)remoteId
                      signalingSender:
                          (id<OWTP2PSignalingSenderProtocol>)signalingSender {
  self = [super init];
  owt::p2p::P2PSignalingSenderInterface* sender =
      new owt::p2p::OWTP2PSignalingSenderObjcImpl(signalingSender);
  _remoteId = remoteId;
  const std::string nativeRemoteId = [remoteId UTF8String];
  const std::string nativeLocalId = [localId UTF8String];
  webrtc::PeerConnectionInterface::IceServers nativeIceServers;
  for (RTCIceServer* server in config.rtcConfiguration.iceServers) {
    nativeIceServers.push_back(server.nativeServer);
  }
  owt::p2p::PeerConnectionChannelConfiguration nativeConfig;
  nativeConfig.servers = nativeIceServers;
  nativeConfig.continual_gathering_policy =
      webrtc::PeerConnectionInterface::GATHER_CONTINUALLY;
  nativeConfig.candidate_network_policy =
      (config.rtcConfiguration.candidateNetworkPolicy ==
       RTCCandidateNetworkPolicyLowCost)
          ? webrtc::PeerConnectionInterface::CandidateNetworkPolicy::
                kCandidateNetworkPolicyLowCost
          : webrtc::PeerConnectionInterface::CandidateNetworkPolicy::
                kCandidateNetworkPolicyAll;
  for (OWTAudioEncodingParameters* audioEncoding in config.audio) {
    nativeConfig.audio.push_back(audioEncoding.nativeAudioEncodingParameters);
  }
  for (OWTVideoEncodingParameters* videoEncoding in config.video) {
    nativeConfig.video.push_back(videoEncoding.nativeVideoEncodingParameters);
  }
  _nativeChannel = new owt::p2p::P2PPeerConnectionChannel(
      nativeConfig, nativeLocalId, nativeRemoteId, sender);
  return self;
}
- (void)publish:(OWTLocalStream*)stream
      onSuccess:(void (^)(OWTP2PPublication*))onSuccess
      onFailure:(void (^)(NSError*))onFailure {
  // Creating PeerConnection(invite-accept) should be handled in native level.
  // Since it is not implemented, we temporary send an invitation here.
  RTCLogInfo(@"OWTP2PPeerConnectionChannel publish stream.");
  _nativeChannel->Publish(
      std::static_pointer_cast<owt::base::LocalStream>([stream nativeStream]),
      [=]() {
        if (onSuccess != nil) {
          OWTP2PPublication* publication =
              [[OWTP2PPublication alloc] initWithStop:^() {
                [self unpublish:stream onSuccess:nil onFailure:nil];
              }
                  stats:^(void (^statsSuccess)(NSArray<RTCLegacyStatsReport*>*),
                          void (^statsFailure)(NSError*)) {
                    [self statsForStream:stream
                               onSuccess:statsSuccess
                               onFailure:statsFailure];
                  }];
          onSuccess(publication);
        }
      },
      [=](std::unique_ptr<owt::base::Exception> e) {
        if (onFailure == nil)
          return;
        NSError* err = [[NSError alloc]
            initWithDomain:OWTErrorDomain
                      code:OWTP2PErrorUnknown
                  userInfo:[[NSDictionary alloc]
                               initWithObjectsAndKeys:
                                   [NSString stringForStdString:e->Message()],
                                   NSLocalizedDescriptionKey, nil]];
        onFailure(err);
      });
}
- (void)unpublish:(OWTLocalStream*)stream
        onSuccess:(void (^)())onSuccess
        onFailure:(void (^)(NSError*))onFailure {
  RTCLogInfo(@"OWTP2PPeerConnectionChannel unpublish stream.");
  _nativeChannel->Unpublish(
      std::static_pointer_cast<owt::base::LocalStream>([stream nativeStream]),
      [=]() {
        if (onSuccess != nil)
          onSuccess();
      },
      [=](std::unique_ptr<owt::base::Exception> e) {
        if (onFailure == nil)
          return;
        NSError* err = [[NSError alloc]
            initWithDomain:OWTErrorDomain
                      code:OWTP2PErrorUnknown
                  userInfo:[[NSDictionary alloc]
                               initWithObjectsAndKeys:
                                   [NSString stringForStdString:e->Message()],
                                   NSLocalizedDescriptionKey, nil]];
        onFailure(err);
      });
}
- (void)send:(NSString*)message
    withOnSuccess:(void (^)())onSuccess
        onFailure:(void (^)(NSError*))onFailure {
  _nativeChannel->Send(
      [message UTF8String],
      [=]() {
        if (onSuccess != nil)
          onSuccess();
      },
      [=](std::unique_ptr<owt::base::Exception> e) {
        if (onFailure == nil)
          return;
        NSError* err = [[NSError alloc]
            initWithDomain:OWTErrorDomain
                      code:OWTP2PErrorUnknown
                  userInfo:[[NSDictionary alloc]
                               initWithObjectsAndKeys:
                                   [NSString stringForStdString:e->Message()],
                                   NSLocalizedDescriptionKey, nil]];
        onFailure(err);
      });
}
- (void)stopWithOnSuccess:(void (^)())onSuccess
                onFailure:(void (^)(NSError*))onFailure {
  _nativeChannel->Stop(
      [=]() {
        if (onSuccess != nil)
          onSuccess();
      },
      [=](std::unique_ptr<owt::base::Exception> e) {
        if (onFailure == nil)
          return;
        NSError* err = [[NSError alloc]
            initWithDomain:OWTErrorDomain
                      code:OWTP2PErrorUnknown
                  userInfo:[[NSDictionary alloc]
                               initWithObjectsAndKeys:
                                   [NSString stringForStdString:e->Message()],
                                   NSLocalizedDescriptionKey, nil]];
        onFailure(err);
      });
}
- (void)statsWithOnSuccess:(void (^)(NSArray<RTCLegacyStatsReport*>*))onSuccess
                 onFailure:(void (^)(NSError*))onFailure {
  _nativeChannel->GetStats(
      [=](const webrtc::StatsReports& reports) {
        if (onSuccess) {
          NSMutableArray* stats =
              [NSMutableArray arrayWithCapacity:reports.size()];
          for (const auto* report : reports) {
            RTCLegacyStatsReport* statsReport =
                [[RTCLegacyStatsReport alloc] initWithNativeReport:*report];
            [stats addObject:statsReport];
          }
          onSuccess(stats);
        }
      },
      [=](std::unique_ptr<owt::base::Exception> e) {
        if (onFailure == nil)
          return;
        NSError* err = [[NSError alloc]
            initWithDomain:OWTErrorDomain
                      code:OWTP2PErrorUnknown
                  userInfo:[[NSDictionary alloc]
                               initWithObjectsAndKeys:
                                   [NSString stringForStdString:e->Message()],
                                   NSLocalizedDescriptionKey, nil]];
        onFailure(err);
      });
}
- (void)statsForStream:(OWTStream*)stream
             onSuccess:(void (^)(NSArray<RTCLegacyStatsReport*>*))onSuccess
             onFailure:(void (^)(NSError*))onFailure {
  _nativeChannel->GetStats(
      [=](const webrtc::StatsReports& reports) {
        if (onSuccess) {
          NSMutableArray* stats =
              [NSMutableArray arrayWithCapacity:reports.size()];
          for (const auto* report : reports) {
            RTCLegacyStatsReport* statsReport =
                [[RTCLegacyStatsReport alloc] initWithNativeReport:*report];
            [stats addObject:statsReport];
          }
          onSuccess(stats);
        }
      },
      [=](std::unique_ptr<owt::base::Exception> e) {
        if (onFailure == nil)
          return;
        NSError* err = [[NSError alloc]
            initWithDomain:OWTErrorDomain
                      code:OWTP2PErrorUnknown
                  userInfo:[[NSDictionary alloc]
                               initWithObjectsAndKeys:
                                   [NSString stringForStdString:e->Message()],
                                   NSLocalizedDescriptionKey, nil]];
        onFailure(err);
      });
}
- (void)onIncomingSignalingMessage:(NSString*)message {
  _nativeChannel->OnIncomingSignalingMessage([message UTF8String]);
}
- (void)addObserver:(id<OWTP2PPeerConnectionChannelObserver>)observer {
  owt::p2p::P2PPeerConnectionChannelObserver* nativeObserver =
      new owt::p2p::P2PPeerConnectionChannelObserverObjcImpl(observer);
  _nativeChannel->AddObserver(nativeObserver);
}
- (void)removeObserver:(id<OWTP2PPeerConnectionChannelObserver>)observer {
}
- (NSString*)getRemoteUserId {
  return _remoteId;
}
@end
