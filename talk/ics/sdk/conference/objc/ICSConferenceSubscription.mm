//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/base/objc/ICSMediaFormat+Private.h"
#import "talk/ics/sdk/conference/objc/ICSConferenceSubscription+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/PeerConnection/RTCLegacyStatsReport+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
#import <ICS/ICSErrors.h>
#import <ICS/ICSConferenceErrors.h>

#include "webrtc/rtc_base/checks.h"

@implementation ICSConferenceSubscription {
  std::shared_ptr<ics::conference::ConferenceSubscription> _nativeSubscription;
}

- (instancetype)initWithNativeSubscription:
    (std::shared_ptr<ics::conference::ConferenceSubscription>)
        nativeSubscription {
  self = [super init];
  _nativeSubscription = nativeSubscription;
  return self;
}

- (void)stop {
  _nativeSubscription->Stop(nullptr, nullptr);
}

- (void)statsWith:(void (^)(NSArray<RTCLegacyStatsReport*>*))onSuccess
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
      [onFailure](std::unique_ptr<ics::base::Exception> e) {
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
      });
}

@end

@implementation ICSConferenceAudioSubscriptionConstraints

- (std::shared_ptr<ics::conference::AudioSubscriptionConstraints>)
    nativeAudioSubscriptionConstraints {
  std::shared_ptr<ics::conference::AudioSubscriptionConstraints> constrains =
      std::shared_ptr<ics::conference::AudioSubscriptionConstraints>(
          new ics::conference::AudioSubscriptionConstraints());
  constrains->codecs =
      std::vector<ics::base::AudioCodecParameters>([_codecs count]);
  for (ICSAudioCodecParameters* codec in _codecs) {
    ics::base::AudioCodecParameters parameters(
        *[codec nativeAudioCodecParameters].get());
    constrains->codecs.push_back(parameters);
  }
  return constrains;
}

@end

@implementation ICSConferenceVideoSubscriptionConstraints

- (std::shared_ptr<ics::conference::VideoSubscriptionConstraints>)
    nativeVideoSubscriptionConstraints {
  std::shared_ptr<ics::conference::VideoSubscriptionConstraints> constrains =
      std::shared_ptr<ics::conference::VideoSubscriptionConstraints>(
          new ics::conference::VideoSubscriptionConstraints());
  constrains->codecs =
      std::vector<ics::base::VideoCodecParameters>([_codecs count]);
  for (ICSVideoCodecParameters* codec in _codecs) {
    ics::base::VideoCodecParameters parameters(
        *[codec nativeVideoCodecParameters].get());
    constrains->codecs.push_back(parameters);
  }
  constrains->resolution =
      ics::base::Resolution(_resolution.width, _resolution.height);
  constrains->frameRate = _frameRate;
  constrains->bitrateMultiplier = _bitrateMultiplier;
  constrains->keyFrameInterval = _keyFrameInterval;
  return constrains;
}

@end

@implementation ICSConferenceSubscribeOptions 

- (instancetype)initWithAudio:(ICSConferenceAudioSubscriptionConstraints*)audio
                        video:
                            (ICSConferenceVideoSubscriptionConstraints*)video {
  if ((self = [super init])) {
    _audio = audio;
    _video = video;
  }
  return self;
}

- (std::shared_ptr<ics::conference::SubscribeOptions>)nativeSubscribeOptions {
  std::shared_ptr<ics::conference::SubscribeOptions> options(
      new ics::conference::SubscribeOptions);
  ics::conference::AudioSubscriptionConstraints audio(
      *[_audio nativeAudioSubscriptionConstraints].get());
  ics::conference::VideoSubscriptionConstraints video(
      *[_video nativeVideoSubscriptionConstraints].get());
  options->audio = audio;
  options->video = video;
  return options;
}

@end
