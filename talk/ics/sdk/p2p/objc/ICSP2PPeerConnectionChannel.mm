/*
 * Intel License
 */

#import "Foundation/Foundation.h"
#import "talk/ics/sdk/include/objc/ICS/ICSErrors.h"
#import "talk/ics/sdk/include/objc/ICS/ICSP2PPeerConnectionChannelObserver.h"
#import "talk/ics/sdk/include/objc/ICS/ICSP2PErrors.h"
#import "talk/ics/sdk/p2p/objc/ICSP2PPeerConnectionChannel.h"
#import "talk/ics/sdk/p2p/objc/P2PPeerConnectionChannelObserverObjcImpl.h"
#import "talk/ics/sdk/p2p/objc/ICSP2PSignalingSenderObjcImpl.h"
#import "talk/ics/sdk/p2p/objc/ICSP2PPublication+Private.h"
#import "talk/ics/sdk/base/objc/ICSLocalStream+Private.h"
#import "talk/ics/sdk/base/objc/ICSMediaFormat+Private.h"
#import "talk/ics/sdk/base/objc/ICSStream+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
#import "webrtc/sdk/objc/Framework/Classes/PeerConnection/RTCIceServer+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/PeerConnection/RTCLegacyStatsReport+Private.h"
#import <WebRTC/RTCLogging.h>

#include "talk/ics/sdk/p2p/p2ppeerconnectionchannel.h"

@implementation ICSP2PPeerConnectionChannel {
  ics::p2p::P2PPeerConnectionChannel* _nativeChannel;
  NSString* _remoteId;
}

- (instancetype)initWithConfiguration:(ICSP2PClientConfiguration*)config
                              localId:(NSString*)localId
                             remoteId:(NSString*)remoteId
                      signalingSender:
                          (id<ICSP2PSignalingSenderProtocol>)signalingSender {
  self = [super init];
  ics::p2p::P2PSignalingSenderInterface* sender =
      new ics::p2p::ICSP2PSignalingSenderObjcImpl(signalingSender);
  _remoteId = remoteId;
  const std::string nativeRemoteId = [remoteId UTF8String];
  const std::string nativeLocalId = [localId UTF8String];
  webrtc::PeerConnectionInterface::IceServers nativeIceServers;
  for (RTCIceServer* server in config.rtcConfiguration.iceServers) {
    nativeIceServers.push_back(server.nativeServer);
  }
  ics::p2p::PeerConnectionChannelConfiguration nativeConfig;
  nativeConfig.servers = nativeIceServers;
  nativeConfig.candidate_network_policy =
      (config.rtcConfiguration.candidateNetworkPolicy ==
       RTCCandidateNetworkPolicyLowCost)
          ? webrtc::PeerConnectionInterface::CandidateNetworkPolicy::
                kCandidateNetworkPolicyLowCost
          : webrtc::PeerConnectionInterface::CandidateNetworkPolicy::
                kCandidateNetworkPolicyAll;
  for (ICSAudioEncodingParameters* audioEncoding in config.audio) {
    nativeConfig.audio.push_back(audioEncoding.nativeAudioEncodingParameters);
  }
  for (ICSVideoEncodingParameters* videoEncoding in config.video) {
    nativeConfig.video.push_back(videoEncoding.nativeVideoEncodingParameters);
  }
  _nativeChannel = new ics::p2p::P2PPeerConnectionChannel(
      nativeConfig, nativeLocalId, nativeRemoteId, sender);
  return self;
}

- (void)publish:(ICSLocalStream*)stream
      onSuccess:(void (^)(ICSP2PPublication*))onSuccess
      onFailure:(void (^)(NSError*))onFailure {
  // Creating PeerConnection(invite-accept) should be handled in native level.
  // Since it is not implemented, we temporary send an invitation here.
  RTCLogInfo(@"ICSP2PPeerConnectionChannel publish stream.");
  _nativeChannel->Publish(
      std::static_pointer_cast<ics::base::LocalStream>([stream nativeStream]),
      [=]() {
        if (onSuccess != nil) {
          ICSP2PPublication* publication =
              [[ICSP2PPublication alloc] initWithStop:^() {
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
      [=](std::unique_ptr<ics::base::Exception> e) {
        if (onFailure == nil)
          return;
        NSError* err = [[NSError alloc]
            initWithDomain:ICSErrorDomain
                      code:ICSP2PErrorUnknown
                  userInfo:[[NSDictionary alloc]
                               initWithObjectsAndKeys:
                                   [NSString stringForStdString:e->Message()],
                                   NSLocalizedDescriptionKey, nil]];
        onFailure(err);
      });
}

- (void)unpublish:(ICSLocalStream*)stream
        onSuccess:(void (^)())onSuccess
        onFailure:(void (^)(NSError*))onFailure {
  RTCLogInfo(@"ICSP2PPeerConnectionChannel unpublish stream.");
  _nativeChannel->Unpublish(
      std::static_pointer_cast<ics::base::LocalStream>([stream nativeStream]),
      [=]() {
        if (onSuccess != nil)
          onSuccess();
      },
      [=](std::unique_ptr<ics::base::Exception> e) {
        if (onFailure == nil)
          return;
        NSError* err = [[NSError alloc]
            initWithDomain:ICSErrorDomain
                      code:ICSP2PErrorUnknown
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
      [=](std::unique_ptr<ics::base::Exception> e) {
        if (onFailure == nil)
          return;
        NSError* err = [[NSError alloc]
            initWithDomain:ICSErrorDomain
                      code:ICSP2PErrorUnknown
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
      [=](std::unique_ptr<ics::base::Exception> e) {
        if (onFailure == nil)
          return;
        NSError* err = [[NSError alloc]
            initWithDomain:ICSErrorDomain
                      code:ICSP2PErrorUnknown
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
      [=](std::unique_ptr<ics::base::Exception> e) {
        if (onFailure == nil)
          return;
        NSError* err = [[NSError alloc]
            initWithDomain:ICSErrorDomain
                      code:ICSP2PErrorUnknown
                  userInfo:[[NSDictionary alloc]
                               initWithObjectsAndKeys:
                                   [NSString stringForStdString:e->Message()],
                                   NSLocalizedDescriptionKey, nil]];
        onFailure(err);
      });
}

- (void)statsForStream:(ICSStream*)stream
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
      [=](std::unique_ptr<ics::base::Exception> e) {
        if (onFailure == nil)
          return;
        NSError* err = [[NSError alloc]
            initWithDomain:ICSErrorDomain
                      code:ICSP2PErrorUnknown
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

- (void)addObserver:(id<ICSP2PPeerConnectionChannelObserver>)observer {
  ics::p2p::P2PPeerConnectionChannelObserver* nativeObserver =
      new ics::p2p::P2PPeerConnectionChannelObserverObjcImpl(observer);
  _nativeChannel->AddObserver(nativeObserver);
}

- (void)removeObserver:(id<ICSP2PPeerConnectionChannelObserver>)observer {
}

- (NSString*)getRemoteUserId {
  return _remoteId;
}

@end
