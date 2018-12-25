// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "talk/oms/sdk/conference/objc/ConferenceSubscriptionObserverObjcImpl.h"
#include "webrtc/rtc_base/checks.h"
#import "talk/oms/sdk/base/objc/OMSMediaFormat+Private.h"
#import "talk/oms/sdk/conference/objc/OMSConferenceSubscription+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/PeerConnection/RTCLegacyStatsReport+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
#import <OMS/OMSErrors.h>
#import <OMS/OMSConferenceErrors.h>
@implementation OMSConferenceSubscription {
  std::shared_ptr<oms::conference::ConferenceSubscription> _nativeSubscription;
  std::unique_ptr<
      oms::conference::ConferenceSubscriptionObserverObjcImpl,
      std::function<void(
          oms::conference::ConferenceSubscriptionObserverObjcImpl*)>>
      _observer;
}
- (instancetype)initWithNativeSubscription:
    (std::shared_ptr<oms::conference::ConferenceSubscription>)
        nativeSubscription {
  self = [super init];
  _nativeSubscription = nativeSubscription;
  return self;
}
- (void)stop {
  _nativeSubscription->Stop();
}
- (void)statsWithOnSuccess:(void (^)(NSArray<RTCLegacyStatsReport*>*))onSuccess
                 onFailure:(nullable void (^)(NSError*))onFailure {
  RTC_CHECK(onSuccess);
  _nativeSubscription->GetNativeStats(
      [onSuccess](const std::vector<const webrtc::StatsReport*>& reports) {
        NSMutableArray* stats =
            [NSMutableArray arrayWithCapacity:reports.size()];
        for (const auto* report : reports) {
          RTCLegacyStatsReport* statsReport =
              [[RTCLegacyStatsReport alloc] initWithNativeReport:*report];
          [stats addObject:statsReport];
        }
        onSuccess(stats);
      },
      [onFailure](std::unique_ptr<oms::base::Exception> e) {
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
      });
}
- (void)mute:(OMSTrackKind)trackKind
    onSuccess:(nullable void (^)())onSuccess
    onFailure:(nullable void (^)(NSError*))onFailure {
  _nativeSubscription->Mute(
      [OMSTrackKindConverter cppTrackKindForObjcTrackKind:trackKind],
      [onSuccess]() {
        if (onSuccess)
          onSuccess();
      },
      [onFailure](std::unique_ptr<oms::base::Exception> e) {
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
      });
}
- (void)unmute:(OMSTrackKind)trackKind
     onSuccess:(nullable void (^)())onSuccess
     onFailure:(nullable void (^)(NSError*))onFailure {
  _nativeSubscription->Unmute(
      [OMSTrackKindConverter cppTrackKindForObjcTrackKind:trackKind],
      [onSuccess]() {
        if (onSuccess)
          onSuccess();
      },
      [onFailure](std::unique_ptr<oms::base::Exception> e) {
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
      });
}
- (void)applyOptions:(OMSConferenceSubscriptionUpdateOptions*)options
           onSuccess:(nullable void (^)())onSuccess
           onFailure:(nullable void (^)(NSError*))onFailure {
  _nativeSubscription->ApplyOptions(
      *[options nativeSubscriptionUpdateOptions].get(),
      [onSuccess]() {
        if (onSuccess)
          onSuccess();
      },
      [onFailure](std::unique_ptr<oms::base::Exception> e) {
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
      });
}
- (NSString*)subscriptionId {
  return [NSString stringForStdString:_nativeSubscription->Id()];
}
-(void)setDelegate:(id<OMSConferenceSubscriptionDelegate>)delegate{
  _observer = std::unique_ptr<
      oms::conference::ConferenceSubscriptionObserverObjcImpl,
      std::function<void(oms::conference::ConferenceSubscriptionObserverObjcImpl*)>>(
      new oms::conference::ConferenceSubscriptionObserverObjcImpl(self, delegate),
      [&self](oms::conference::ConferenceSubscriptionObserverObjcImpl* observer) {
        self->_nativeSubscription->RemoveObserver(*observer);
      });
  _nativeSubscription->AddObserver(*_observer.get());
  _delegate = delegate;
}
@end
@implementation OMSConferenceAudioSubscriptionConstraints
- (std::shared_ptr<oms::conference::AudioSubscriptionConstraints>)
    nativeAudioSubscriptionConstraints {
  std::shared_ptr<oms::conference::AudioSubscriptionConstraints> constrains =
      std::shared_ptr<oms::conference::AudioSubscriptionConstraints>(
          new oms::conference::AudioSubscriptionConstraints());
  constrains->disabled = _disabled;
  constrains->codecs =
      std::vector<oms::base::AudioCodecParameters>([_codecs count]);
  for (OMSAudioCodecParameters* codec in _codecs) {
    oms::base::AudioCodecParameters parameters(
        *[codec nativeAudioCodecParameters].get());
    constrains->codecs.push_back(parameters);
  }
  return constrains;
}
@end
@implementation OMSConferenceVideoSubscriptionConstraints
- (std::shared_ptr<oms::conference::VideoSubscriptionConstraints>)
    nativeVideoSubscriptionConstraints {
  std::shared_ptr<oms::conference::VideoSubscriptionConstraints> constrains =
      std::shared_ptr<oms::conference::VideoSubscriptionConstraints>(
          new oms::conference::VideoSubscriptionConstraints());
  constrains->disabled = _disabled;
  constrains->codecs =
      std::vector<oms::base::VideoCodecParameters>([_codecs count]);
  for (OMSVideoCodecParameters* codec in _codecs) {
    oms::base::VideoCodecParameters parameters(
        *[codec nativeVideoCodecParameters].get());
    constrains->codecs.push_back(parameters);
  }
  constrains->resolution =
      oms::base::Resolution(_resolution.width, _resolution.height);
  constrains->frameRate = _frameRate;
  constrains->bitrateMultiplier = _bitrateMultiplier;
  constrains->keyFrameInterval = _keyFrameInterval;
  return constrains;
}
@end
@implementation OMSConferenceSubscribeOptions
- (instancetype)initWithAudio:(OMSConferenceAudioSubscriptionConstraints*)audio
                        video:
                            (OMSConferenceVideoSubscriptionConstraints*)video {
  if ((self = [super init])) {
    _audio = audio;
    _video = video;
  }
  return self;
}
- (std::shared_ptr<oms::conference::SubscribeOptions>)nativeSubscribeOptions {
  std::shared_ptr<oms::conference::SubscribeOptions> options(
      new oms::conference::SubscribeOptions);
  if (_audio) {
    oms::conference::AudioSubscriptionConstraints audio(
        *[_audio nativeAudioSubscriptionConstraints].get());
    options->audio = audio;
  }
  if (_video) {
    oms::conference::VideoSubscriptionConstraints video(
        *[_video nativeVideoSubscriptionConstraints].get());
    options->video = video;
  }
  return options;
}
@end

@implementation OMSConferenceVideoSubscriptionUpdateConstraints
- (std::shared_ptr<oms::conference::VideoSubscriptionUpdateConstraints>)
    nativeVideoSubscriptionUpdateConstraints {
  std::shared_ptr<oms::conference::VideoSubscriptionUpdateConstraints> constrains =
      std::shared_ptr<oms::conference::VideoSubscriptionUpdateConstraints>(
          new oms::conference::VideoSubscriptionUpdateConstraints());
  constrains->resolution =
      oms::base::Resolution(_resolution.width, _resolution.height);
  constrains->frameRate = _frameRate;
  constrains->bitrateMultiplier = _bitrateMultiplier;
  constrains->keyFrameInterval = _keyFrameInterval;
  return constrains;
}
@end
@implementation OMSConferenceSubscriptionUpdateOptions
- (std::shared_ptr<oms::conference::SubscriptionUpdateOptions>)
    nativeSubscriptionUpdateOptions {
  std::shared_ptr<oms::conference::SubscriptionUpdateOptions> options =
      std::shared_ptr<oms::conference::SubscriptionUpdateOptions>(
          new oms::conference::SubscriptionUpdateOptions());
  if (_video) {
    oms::conference::VideoSubscriptionUpdateConstraints video(
        *[_video nativeVideoSubscriptionUpdateConstraints].get());
    options->video = video;
  }
  return options;
}
@end
