// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "Foundation/Foundation.h"
#import "talk/oms/sdk/include/objc/OMS/OMSErrors.h"
#import "talk/oms/sdk/include/objc/OMS/OMSP2PPeerConnectionChannelObserver.h"
#import "talk/oms/sdk/include/objc/OMS/OMSP2PErrors.h"
#import "talk/oms/sdk/p2p/objc/OMSP2PPeerConnectionChannel.h"
#import "talk/oms/sdk/p2p/objc/P2PPeerConnectionChannelObserverObjcImpl.h"
#import "talk/oms/sdk/p2p/objc/OMSP2PSignalingSenderObjcImpl.h"
#import "talk/oms/sdk/p2p/objc/OMSP2PPublication+Private.h"
#import "talk/oms/sdk/base/objc/OMSLocalStream+Private.h"
#import "talk/oms/sdk/base/objc/OMSMediaFormat+Private.h"
#import "talk/oms/sdk/base/objc/OMSStream+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
#import "webrtc/sdk/objc/Framework/Classes/PeerConnection/RTCIceServer+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/PeerConnection/RTCLegacyStatsReport+Private.h"
#import <WebRTC/RTCLogging.h>
#include "talk/oms/sdk/p2p/p2ppeerconnectionchannel.h"
@implementation OMSP2PPeerConnectionChannel {
  oms::p2p::P2PPeerConnectionChannel* _nativeChannel;
  NSString* _remoteId;
}
- (instancetype)initWithConfiguration:(OMSP2PClientConfiguration*)config
                              localId:(NSString*)localId
                             remoteId:(NSString*)remoteId
                      signalingSender:
                          (id<OMSP2PSignalingSenderProtocol>)signalingSender {
  self = [super init];
  oms::p2p::P2PSignalingSenderInterface* sender =
      new oms::p2p::OMSP2PSignalingSenderObjcImpl(signalingSender);
  _remoteId = remoteId;
  const std::string nativeRemoteId = [remoteId UTF8String];
  const std::string nativeLocalId = [localId UTF8String];
  webrtc::PeerConnectionInterface::IceServers nativeIceServers;
  for (RTCIceServer* server in config.rtcConfiguration.iceServers) {
    nativeIceServers.push_back(server.nativeServer);
  }
  oms::p2p::PeerConnectionChannelConfiguration nativeConfig;
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
  for (OMSAudioEncodingParameters* audioEncoding in config.audio) {
    nativeConfig.audio.push_back(audioEncoding.nativeAudioEncodingParameters);
  }
  for (OMSVideoEncodingParameters* videoEncoding in config.video) {
    nativeConfig.video.push_back(videoEncoding.nativeVideoEncodingParameters);
  }
  _nativeChannel = new oms::p2p::P2PPeerConnectionChannel(
      nativeConfig, nativeLocalId, nativeRemoteId, sender);
  return self;
}
- (void)publish:(OMSLocalStream*)stream
      onSuccess:(void (^)(OMSP2PPublication*))onSuccess
      onFailure:(void (^)(NSError*))onFailure {
  // Creating PeerConnection(invite-accept) should be handled in native level.
  // Since it is not implemented, we temporary send an invitation here.
  RTCLogInfo(@"OMSP2PPeerConnectionChannel publish stream.");
  _nativeChannel->Publish(
      std::static_pointer_cast<oms::base::LocalStream>([stream nativeStream]),
      [=]() {
        if (onSuccess != nil) {
          OMSP2PPublication* publication =
              [[OMSP2PPublication alloc] initWithStop:^() {
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
      [=](std::unique_ptr<oms::base::Exception> e) {
        if (onFailure == nil)
          return;
        NSError* err = [[NSError alloc]
            initWithDomain:OMSErrorDomain
                      code:OMSP2PErrorUnknown
                  userInfo:[[NSDictionary alloc]
                               initWithObjectsAndKeys:
                                   [NSString stringForStdString:e->Message()],
                                   NSLocalizedDescriptionKey, nil]];
        onFailure(err);
      });
}
- (void)unpublish:(OMSLocalStream*)stream
        onSuccess:(void (^)())onSuccess
        onFailure:(void (^)(NSError*))onFailure {
  RTCLogInfo(@"OMSP2PPeerConnectionChannel unpublish stream.");
  _nativeChannel->Unpublish(
      std::static_pointer_cast<oms::base::LocalStream>([stream nativeStream]),
      [=]() {
        if (onSuccess != nil)
          onSuccess();
      },
      [=](std::unique_ptr<oms::base::Exception> e) {
        if (onFailure == nil)
          return;
        NSError* err = [[NSError alloc]
            initWithDomain:OMSErrorDomain
                      code:OMSP2PErrorUnknown
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
      [=](std::unique_ptr<oms::base::Exception> e) {
        if (onFailure == nil)
          return;
        NSError* err = [[NSError alloc]
            initWithDomain:OMSErrorDomain
                      code:OMSP2PErrorUnknown
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
      [=](std::unique_ptr<oms::base::Exception> e) {
        if (onFailure == nil)
          return;
        NSError* err = [[NSError alloc]
            initWithDomain:OMSErrorDomain
                      code:OMSP2PErrorUnknown
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
      [=](std::unique_ptr<oms::base::Exception> e) {
        if (onFailure == nil)
          return;
        NSError* err = [[NSError alloc]
            initWithDomain:OMSErrorDomain
                      code:OMSP2PErrorUnknown
                  userInfo:[[NSDictionary alloc]
                               initWithObjectsAndKeys:
                                   [NSString stringForStdString:e->Message()],
                                   NSLocalizedDescriptionKey, nil]];
        onFailure(err);
      });
}
- (void)statsForStream:(OMSStream*)stream
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
      [=](std::unique_ptr<oms::base::Exception> e) {
        if (onFailure == nil)
          return;
        NSError* err = [[NSError alloc]
            initWithDomain:OMSErrorDomain
                      code:OMSP2PErrorUnknown
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
- (void)addObserver:(id<OMSP2PPeerConnectionChannelObserver>)observer {
  oms::p2p::P2PPeerConnectionChannelObserver* nativeObserver =
      new oms::p2p::P2PPeerConnectionChannelObserverObjcImpl(observer);
  _nativeChannel->AddObserver(nativeObserver);
}
- (void)removeObserver:(id<OMSP2PPeerConnectionChannelObserver>)observer {
}
- (NSString*)getRemoteUserId {
  return _remoteId;
}
@end
