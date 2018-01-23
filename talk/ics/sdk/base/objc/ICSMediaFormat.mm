//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "talk/ics/sdk/base/objc/ICSMediaFormat+Private.h"
#import "webrtc/rtc_base/checks.h"

#include "talk/ics/sdk/include/cpp/ics/base/options.h"

@implementation ICSVideoFormat {
  ics::base::VideoFormat* _videoFormat;
}

- (instancetype)init {
  self = [super init];
  ics::base::Resolution resolution(0, 0);
  _videoFormat = new ics::base::VideoFormat(resolution);
  return self;
}

- (void)dealloc {
  delete _videoFormat;
}

- (CGSize)resolution {
  return CGSizeMake(_videoFormat->resolution.width,
                    _videoFormat->resolution.height);
}

- (instancetype)initWithNativeVideoFormat:
    (const ics::base::VideoFormat&)videoFormat {
  self = [super init];
  ics::base::Resolution resolution(videoFormat.resolution.width,
                                   videoFormat.resolution.height);
  _videoFormat = new ics::base::VideoFormat(resolution);
  return self;
}

- (NSString*)description {
  return [NSString stringWithFormat:@"Resolution: %f x %f",
                                    [self resolution].width,
                                    [self resolution].height];
}

@end

@implementation ICSAudioCodecParameters

@dynamic name, channelCount, clockRate;

static std::unordered_map<ICSAudioCodec, const ics::base::AudioCodec>
    audioCodecMap = {{ICSAudioCodecOpus, ics::base::AudioCodec::kOpus},
                     {ICSAudioCodecIsac, ics::base::AudioCodec::kIsac},
                     {ICSAudioCodecG722, ics::base::AudioCodec::kG722},
                     {ICSAudioCodecPcmu, ics::base::AudioCodec::kPcmu},
                     {ICSAudioCodecPcma, ics::base::AudioCodec::kPcma},
                     {ICSAudioCodecIlbc, ics::base::AudioCodec::kIlbc},
                     {ICSAudioCodecAac, ics::base::AudioCodec::kAac},
                     {ICSAudioCodecAc3, ics::base::AudioCodec::kAc3},
                     {ICSAudioCodecAsao, ics::base::AudioCodec::kAsao},
                     {ICSAudioCodecUnknown, ics::base::AudioCodec::kUnknown}};

- (instancetype)initWithNativeAudioCodecParameters:
    (const ics::base::AudioCodecParameters&)nativeAudioCodecParameters {
  if (self = [super init]) {
    _nativeAudioCodecParameters = nativeAudioCodecParameters;
  }
  return self;
}

- (ICSAudioCodec)name {
  auto it = std::find_if(
      audioCodecMap.begin(), audioCodecMap.end(), [self](auto&& codec) {
        return codec.second == self.nativeAudioCodecParameters.name;
      });
  if (it != audioCodecMap.end()) {
    return it->first;
  } else {
    RTC_NOTREACHED();
    return ICSAudioCodecUnknown;
  }
}

- (NSUInteger)channelCount {
  return _nativeAudioCodecParameters.channel_count;
}

- (NSUInteger)clockRate {
  return _nativeAudioCodecParameters.clock_rate;
}

@end

@implementation ICSVideoCodecParameters

@dynamic name, profile;

static std::unordered_map<ICSVideoCodec, const ics::base::VideoCodec>
    videoCodecMap = {{ICSVideoCodecVP8, ics::base::VideoCodec::kVp8},
                     {ICSVideoCodecVP9, ics::base::VideoCodec::kVp9},
                     {ICSVideoCodecH264, ics::base::VideoCodec::kH264},
                     {ICSVideoCodecH265, ics::base::VideoCodec::kH265}};

- (instancetype)initWithNativeVideoCodecParameters:
    (const ics::base::VideoCodecParameters&)nativeVideoCodecParameters {
  if (self = [super init]) {
    _nativeVideoCodecParameters = nativeVideoCodecParameters;
  }
  return self;
}

- (ICSVideoCodec)name {
  auto it = std::find_if(
      videoCodecMap.begin(), videoCodecMap.end(), [self](auto&& codec) {
        return codec.second == self.nativeVideoCodecParameters.name;
      });
  if (it != videoCodecMap.end()) {
    return it->first;
  } else {
    RTC_NOTREACHED();
    return ICSVideoCodecUnknown;
  }
}

@end

@implementation ICSPublicationSettings

- (instancetype)initWithNativePublicationSettings:
    (const ics::base::PublicationSettings &)nativeSettings {
  if (self = [super init]) {
    _nativeSettings = nativeSettings;
  }
  _audio = [[ICSAudioPublicationSettings alloc]
      initWithNativeAudioPublicationSettings:nativeSettings.audio];
  _video = [[ICSVideoPublicationSettings alloc]
      initWithNativeVideoPublicationSettings:nativeSettings.video];
  return self;
}

@end

@implementation ICSAudioPublicationSettings

@dynamic codec;

- (instancetype)initWithNativeAudioPublicationSettings:
    (const ics::base::AudioPublicationSettings &)nativeSettings {
  if (self = [super init]) {
    _nativeSettings = nativeSettings;
  }
  return self;
}

- (ICSAudioCodecParameters*)codec {
  return [[ICSAudioCodecParameters alloc]
      initWithNativeAudioCodecParameters:_nativeSettings.codec];
}

@end

@implementation ICSVideoPublicationSettings

@dynamic codec, resolution, frameRate, bitrate, keyframeInterval;

- (instancetype)initWithNativeVideoPublicationSettings:
    (const ics::base::VideoPublicationSettings &)nativeSettings {
  if (self = [super init]) {
    _nativeSettings = nativeSettings;
  }
  return self;
}

- (ICSVideoCodecParameters*)codec {
  return [[ICSVideoCodecParameters alloc]
      initWithNativeVideoCodecParameters:_nativeSettings.codec];
}

- (CGSize)resolution {
  return CGSizeMake(_nativeSettings.resolution.width,
                    _nativeSettings.resolution.height);
}

-(CGFloat)frameRate{
  return _nativeSettings.frame_rate;
}

-(NSUInteger)bitrate{
  return _nativeSettings.bitrate;
}

-(NSUInteger)keyframeInterval{
  return _nativeSettings.keyframe_interval;
}

@end

@implementation ICSAudioSubscriptionCapabilities

@dynamic codecs;

- (instancetype)initWithNativeAudioSubscriptionCapabilities:
    (const ics::base::AudioSubscriptionCapabilities&)nativeCapabilities {
  if (self = [super init]) {
    _nativeCapabilities = nativeCapabilities;
  }
  return self;
}

- (NSArray<ICSAudioCodecParameters*>*)codecs {
  NSMutableArray<ICSAudioCodecParameters*>* codecArray =
      [NSMutableArray arrayWithCapacity:_nativeCapabilities.codecs.size()];
  for (auto& c : _nativeCapabilities.codecs) {
    ICSAudioCodecParameters* p =
        [[ICSAudioCodecParameters alloc] initWithNativeAudioCodecParameters:c];
    [codecArray addObject:p];
  }
  return codecArray;
}

@end

@implementation ICSVideoSubscriptionCapabilities

@dynamic codecs, resolutions, frameRates;

- (instancetype)initWithNativeVideoSubscriptionCapabilities:
    (const ics::base::VideoSubscriptionCapabilities&)nativeCapabilities {
  if (self = [super init]) {
    _nativeCapabilities = nativeCapabilities;
  }
  return self;
}

- (NSArray<ICSVideoCodecParameters*>*)codecs {
  NSMutableArray<ICSVideoCodecParameters*>* codecArray =
      [NSMutableArray arrayWithCapacity:_nativeCapabilities.codecs.size()];
  for (const auto& c : _nativeCapabilities.codecs) {
    ICSVideoCodecParameters* p =
        [[ICSVideoCodecParameters alloc] initWithNativeVideoCodecParameters:c];
    [codecArray addObject:p];
  }
  return codecArray;
}

- (NSArray<NSValue*>*)resolutions {
  NSMutableArray<NSValue*>* values =
      [NSMutableArray arrayWithCapacity:_nativeCapabilities.resolutions.size()];
  for (const auto& r : _nativeCapabilities.resolutions) {
    NSValue* v = [NSValue valueWithCGSize:CGSizeMake(r.width, r.height)];
    [values addObject:v];
  }
  return values;
}

- (NSArray<NSNumber*>*)frameRates {
  NSMutableArray<NSNumber*>* values =
      [NSMutableArray arrayWithCapacity:_nativeCapabilities.frame_rates.size()];
  for (const auto& f : _nativeCapabilities.frame_rates) {
    NSNumber* v = [NSNumber numberWithFloat:f];
    [values addObject:v];
  }
  return values;
}

@end


@implementation ICSSubscriptionCapabilities

- (instancetype)initWithNativeSubscriptionCapabilities:
    (const ics::base::SubscriptionCapabilities&)nativeCapabilities {
  if (self = [super init]) {
    _nativeCapabilities = nativeCapabilities;
    _audio = [[ICSAudioSubscriptionCapabilities alloc]
        initWithNativeAudioSubscriptionCapabilities:nativeCapabilities.audio];
    _video = [[ICSVideoSubscriptionCapabilities alloc]
        initWithNativeVideoSubscriptionCapabilities:nativeCapabilities.video];
  }
  return self;
}

@end
