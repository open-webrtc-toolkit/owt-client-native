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
#import "talk/ics/sdk/base/objc/ICSConnectionStats+Internal.h"
#import "talk/ics/sdk/base/objc/ICSLocalStream+Private.h"
#import "talk/ics/sdk/base/objc/ICSMediaFormat+Private.h"
#import "talk/ics/sdk/base/objc/ICSStream+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
#import "webrtc/sdk/objc/Framework/Classes/PeerConnection/RTCIceServer+Private.h"
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
  for (RTCIceServer* server in config.ICEServers) {
    nativeIceServers.push_back(server.nativeServer);
  }
  ics::p2p::PeerConnectionChannelConfiguration nativeConfig;
  nativeConfig.servers = nativeIceServers;
  //nativeConfig.max_audio_bandwidth = [config maxAudioBandwidth];
  //nativeConfig.max_video_bandwidth = [config maxVideoBandwidth];
  /*
  nativeConfig.media_codec.audio_codec =
      [RTCMediaCodec nativeAudioCodec:config.mediaCodec.audioCodec];
  nativeConfig.media_codec.video_codec =
      [RTCMediaCodec nativeVideoCodec:config.mediaCodec.videoCodec];*/
  nativeConfig.candidate_network_policy =
      ([config candidateNetworkPolicy] == RTCCandidateNetworkPolicyLowCost)
          ? webrtc::PeerConnectionInterface::CandidateNetworkPolicy::
                kCandidateNetworkPolicyLowCost
          : webrtc::PeerConnectionInterface::CandidateNetworkPolicy::
                kCandidateNetworkPolicyAll;
  _nativeChannel = new ics::p2p::P2PPeerConnectionChannel(
      nativeConfig, nativeLocalId, nativeRemoteId, sender);
  return self;
}

- (void)inviteWithOnSuccess:(void (^)())onSuccess
                  onFailure:(void (^)(NSError*))onFailure {
  _nativeChannel->Invite(
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

- (void)denyWithOnSuccess:(void (^)())onSuccess
                onFailure:(void (^)(NSError*))onFailure {
  _nativeChannel->Deny(
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

- (void)acceptWithOnSuccess:(void (^)())onSuccess
                  onFailure:(void (^)(NSError*))onFailure {
  _nativeChannel->Accept(
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

- (void)publish:(ICSLocalStream*)stream
      onSuccess:(void (^)(ICSP2PPublication*))onSuccess
      onFailure:(void (^)(NSError*))onFailure {
  // Creating PeerConnection(invite-accept) should be handled in native level.
  // Since it is not implemented, we temporary send an invitation here.
  [self inviteWithOnSuccess:^() {
    RTCLogInfo(@"ICSP2PPeerConnectionChannel publish stream.");
    _nativeChannel->Publish(
        std::static_pointer_cast<ics::base::LocalStream>([stream nativeStream]),
        [=]() {
          if (onSuccess != nil) {
            ICSP2PPublication* publication =
                [[ICSP2PPublication alloc] initWithStop:^() {
                  [self unpublish:stream onSuccess:nil onFailure:nil];
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
                  onFailure:onFailure];
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

- (void)getConnectionStatsWithOnSuccess:(void (^)(ICSConnectionStats*))onSuccess
                              onFailure:(void (^)(NSError*))onFailure {
  _nativeChannel->GetConnectionStats(
      [=](std::shared_ptr<ics::base::ConnectionStats> native_stats) {
        if (onSuccess) {
          onSuccess([[ICSConnectionStats alloc]
              initWithNativeStats:*native_stats.get()]);
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
