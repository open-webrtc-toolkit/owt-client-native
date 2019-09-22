// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "talk/owt/sdk/conference/objc/ConferenceSubscriptionObserverObjcImpl.h"
#include "webrtc/rtc_base/checks.h"
#import "talk/owt/sdk/base/objc/OWTMediaFormat+Private.h"
#import "talk/owt/sdk/conference/objc/OWTConferenceSubscription+Private.h"
#import "webrtc/sdk/objc/api/peerconnection/RTCLegacyStatsReport+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
#import <OWT/OWTErrors.h>
#import <OWT/OWTConferenceErrors.h>
@implementation OWTConferenceSubscription {
  std::shared_ptr<owt::conference::ConferenceSubscription> _nativeSubscription;
  std::unique_ptr<
      owt::conference::ConferenceSubscriptionObserverObjcImpl,
      std::function<void(
          owt::conference::ConferenceSubscriptionObserverObjcImpl*)>>
      _observer;
}
- (instancetype)initWithNativeSubscription:
    (std::shared_ptr<owt::conference::ConferenceSubscription>)
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
      [onFailure](std::unique_ptr<owt::base::Exception> e) {
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
      });
}
- (void)mute:(OWTTrackKind)trackKind
    onSuccess:(nullable void (^)())onSuccess
    onFailure:(nullable void (^)(NSError*))onFailure {
  _nativeSubscription->Mute(
      [OWTTrackKindConverter cppTrackKindForObjcTrackKind:trackKind],
      [onSuccess]() {
        if (onSuccess)
          onSuccess();
      },
      [onFailure](std::unique_ptr<owt::base::Exception> e) {
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
      });
}
- (void)unmute:(OWTTrackKind)trackKind
     onSuccess:(nullable void (^)())onSuccess
     onFailure:(nullable void (^)(NSError*))onFailure {
  _nativeSubscription->Unmute(
      [OWTTrackKindConverter cppTrackKindForObjcTrackKind:trackKind],
      [onSuccess]() {
        if (onSuccess)
          onSuccess();
      },
      [onFailure](std::unique_ptr<owt::base::Exception> e) {
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
      });
}
- (void)applyOptions:(OWTConferenceSubscriptionUpdateOptions*)options
           onSuccess:(nullable void (^)())onSuccess
           onFailure:(nullable void (^)(NSError*))onFailure {
  _nativeSubscription->ApplyOptions(
      *[options nativeSubscriptionUpdateOptions].get(),
      [onSuccess]() {
        if (onSuccess)
          onSuccess();
      },
      [onFailure](std::unique_ptr<owt::base::Exception> e) {
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
      });
}
- (NSString*)subscriptionId {
  return [NSString stringForStdString:_nativeSubscription->Id()];
}
-(void)setDelegate:(id<OWTConferenceSubscriptionDelegate>)delegate{
  if (delegate != nil) {
    __weak OWTConferenceSubscription *weakSelf = self;
    _observer = std::unique_ptr<
                owt::conference::ConferenceSubscriptionObserverObjcImpl,
            std::function<void(owt::conference::ConferenceSubscriptionObserverObjcImpl*)>>(
                    new owt::conference::ConferenceSubscriptionObserverObjcImpl(self, delegate),
                            [=](owt::conference::ConferenceSubscriptionObserverObjcImpl* observer) {
                                __strong OWTConferenceSubscription *strongSelf = weakSelf;
                                if (strongSelf) {
                                  strongSelf->_nativeSubscription->RemoveObserver(*observer);
                                }
                                delete observer;
                            });
    _nativeSubscription->AddObserver(*_observer.get());
  } else {
    _observer.reset();
  }
  _delegate = delegate;
}
@end
@implementation OWTConferenceAudioSubscriptionConstraints
- (std::shared_ptr<owt::conference::AudioSubscriptionConstraints>)
    nativeAudioSubscriptionConstraints {
  std::shared_ptr<owt::conference::AudioSubscriptionConstraints> constrains =
      std::shared_ptr<owt::conference::AudioSubscriptionConstraints>(
          new owt::conference::AudioSubscriptionConstraints());
  constrains->disabled = _disabled;
  constrains->codecs =
      std::vector<owt::base::AudioCodecParameters>([_codecs count]);
  for (OWTAudioCodecParameters* codec in _codecs) {
    owt::base::AudioCodecParameters parameters(
        *[codec nativeAudioCodecParameters].get());
    constrains->codecs.push_back(parameters);
  }
  return constrains;
}
@end
@implementation OWTConferenceVideoSubscriptionConstraints
- (std::shared_ptr<owt::conference::VideoSubscriptionConstraints>)
    nativeVideoSubscriptionConstraints {
  std::shared_ptr<owt::conference::VideoSubscriptionConstraints> constrains =
      std::shared_ptr<owt::conference::VideoSubscriptionConstraints>(
          new owt::conference::VideoSubscriptionConstraints());
  constrains->disabled = _disabled;
  constrains->codecs =
      std::vector<owt::base::VideoCodecParameters>([_codecs count]);
  for (OWTVideoCodecParameters* codec in _codecs) {
    owt::base::VideoCodecParameters parameters(
        *[codec nativeVideoCodecParameters].get());
    constrains->codecs.push_back(parameters);
  }
  constrains->resolution =
      owt::base::Resolution(_resolution.width, _resolution.height);
  constrains->frameRate = _frameRate;
  constrains->bitrateMultiplier = _bitrateMultiplier;
  constrains->keyFrameInterval = _keyFrameInterval;
  constrains->rid = [NSString stdStringForString:_rid];
  return constrains;
}
@end
@implementation OWTConferenceSubscribeOptions
- (instancetype)initWithAudio:(OWTConferenceAudioSubscriptionConstraints*)audio
                        video:
                            (OWTConferenceVideoSubscriptionConstraints*)video {
  if ((self = [super init])) {
    _audio = audio;
    _video = video;
  }
  return self;
}
- (std::shared_ptr<owt::conference::SubscribeOptions>)nativeSubscribeOptions {
  std::shared_ptr<owt::conference::SubscribeOptions> options(
      new owt::conference::SubscribeOptions);
  if (_audio) {
    owt::conference::AudioSubscriptionConstraints audio(
        *[_audio nativeAudioSubscriptionConstraints].get());
    options->audio = audio;
  }
  if (_video) {
    owt::conference::VideoSubscriptionConstraints video(
        *[_video nativeVideoSubscriptionConstraints].get());
    options->video = video;
  }
  return options;
}
@end

@implementation OWTConferenceVideoSubscriptionUpdateConstraints
- (std::shared_ptr<owt::conference::VideoSubscriptionUpdateConstraints>)
    nativeVideoSubscriptionUpdateConstraints {
  std::shared_ptr<owt::conference::VideoSubscriptionUpdateConstraints> constrains =
      std::shared_ptr<owt::conference::VideoSubscriptionUpdateConstraints>(
          new owt::conference::VideoSubscriptionUpdateConstraints());
  constrains->resolution =
      owt::base::Resolution(_resolution.width, _resolution.height);
  constrains->frameRate = _frameRate;
  constrains->bitrateMultiplier = _bitrateMultiplier;
  constrains->keyFrameInterval = _keyFrameInterval;
  return constrains;
}
@end
@implementation OWTConferenceSubscriptionUpdateOptions
- (std::shared_ptr<owt::conference::SubscriptionUpdateOptions>)
    nativeSubscriptionUpdateOptions {
  std::shared_ptr<owt::conference::SubscriptionUpdateOptions> options =
      std::shared_ptr<owt::conference::SubscriptionUpdateOptions>(
          new owt::conference::SubscriptionUpdateOptions());
  if (_video) {
    owt::conference::VideoSubscriptionUpdateConstraints video(
        *[_video nativeVideoSubscriptionUpdateConstraints].get());
    options->video = video;
  }
  return options;
}
@end
