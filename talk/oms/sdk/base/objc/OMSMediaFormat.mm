//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "talk/oms/sdk/base/objc/OMSMediaFormat+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"

#include "talk/oms/sdk/include/cpp/oms/base/options.h"
#include "webrtc/rtc_base/checks.h"

@implementation OMSAudioCodecParameters

@dynamic nativeAudioCodecParameters;

static std::unordered_map<OMSAudioCodec, const oms::base::AudioCodec>
    audioCodecMap = {{OMSAudioCodecOpus, oms::base::AudioCodec::kOpus},
                     {OMSAudioCodecIsac, oms::base::AudioCodec::kIsac},
                     {OMSAudioCodecG722, oms::base::AudioCodec::kG722},
                     {OMSAudioCodecPcmu, oms::base::AudioCodec::kPcmu},
                     {OMSAudioCodecPcma, oms::base::AudioCodec::kPcma},
                     {OMSAudioCodecIlbc, oms::base::AudioCodec::kIlbc},
                     {OMSAudioCodecAac, oms::base::AudioCodec::kAac},
                     {OMSAudioCodecAc3, oms::base::AudioCodec::kAc3},
                     {OMSAudioCodecAsao, oms::base::AudioCodec::kAsao},
                     {OMSAudioCodecUnknown, oms::base::AudioCodec::kUnknown}};

- (instancetype)initWithNativeAudioCodecParameters:
    (const oms::base::AudioCodecParameters&)nativeAudioCodecParameters {
  if (self = [super init]) {
    auto it =
        std::find_if(audioCodecMap.begin(), audioCodecMap.end(),
                     [&nativeAudioCodecParameters](auto&& codec) {
                       return codec.second == nativeAudioCodecParameters.name;
                     });
    if (it != audioCodecMap.end()) {
      _name = it->first;
    } else {
      RTC_NOTREACHED();
      _name = OMSAudioCodecUnknown;
    }
    _channelCount = nativeAudioCodecParameters.channel_count;
    _clockRate = nativeAudioCodecParameters.clock_rate;
  }
  return self;
}

- (std::shared_ptr<oms::base::AudioCodecParameters>)nativeAudioCodecParameters {
  oms::base::AudioCodec codec_name = audioCodecMap[self.name];
  auto nativeParameters = std::shared_ptr<oms::base::AudioCodecParameters>(
      new oms::base::AudioCodecParameters(codec_name, self.channelCount,
                                          self.clockRate));
  return nativeParameters;
}

@end

@implementation OMSVideoCodecParameters

@dynamic nativeVideoCodecParameters;

static std::unordered_map<OMSVideoCodec, const oms::base::VideoCodec>
    videoCodecMap = {{OMSVideoCodecVP8, oms::base::VideoCodec::kVp8},
                     {OMSVideoCodecVP9, oms::base::VideoCodec::kVp9},
                     {OMSVideoCodecH264, oms::base::VideoCodec::kH264},
                     {OMSVideoCodecH265, oms::base::VideoCodec::kH265}};

- (instancetype)initWithNativeVideoCodecParameters:
    (const oms::base::VideoCodecParameters&)nativeVideoCodecParameters {
  if (self = [super init]) {
    auto it =
        std::find_if(videoCodecMap.begin(), videoCodecMap.end(),
                     [&nativeVideoCodecParameters](auto&& codec) {
                       return codec.second == nativeVideoCodecParameters.name;
                     });
    if (it != videoCodecMap.end()) {
      _name = it->first;
    } else {
      RTC_NOTREACHED();
      _name = OMSVideoCodecUnknown;
    }
    _profile =
        [NSString stringForStdString:nativeVideoCodecParameters.profile];
  }
  return self;
}

- (std::shared_ptr<oms::base::VideoCodecParameters>)nativeVideoCodecParameters {
  oms::base::VideoCodec codec_name = videoCodecMap[self.name];
  auto nativeVideoCodecParameters =
      std::shared_ptr<oms::base::VideoCodecParameters>(
          new oms::base::VideoCodecParameters(
              codec_name, [NSString stdStringForString:self.profile]));
  return nativeVideoCodecParameters;
}

@end

@implementation OMSPublicationSettings

- (instancetype)initWithNativePublicationSettings:
    (const oms::base::PublicationSettings &)nativeSettings {
  if (self = [super init]) {
    _nativeSettings = nativeSettings;
  }
  _audio = [[OMSAudioPublicationSettings alloc]
      initWithNativeAudioPublicationSettings:nativeSettings.audio];
  _video = [[OMSVideoPublicationSettings alloc]
      initWithNativeVideoPublicationSettings:nativeSettings.video];
  return self;
}

@end

@implementation OMSAudioPublicationSettings

@dynamic codec;

- (instancetype)initWithNativeAudioPublicationSettings:
    (const oms::base::AudioPublicationSettings &)nativeSettings {
  if (self = [super init]) {
    _nativeSettings = nativeSettings;
  }
  return self;
}

- (OMSAudioCodecParameters*)codec {
  return [[OMSAudioCodecParameters alloc]
      initWithNativeAudioCodecParameters:_nativeSettings.codec];
}

@end

@implementation OMSVideoPublicationSettings

@dynamic codec, resolution, frameRate, bitrate, keyframeInterval;

- (instancetype)initWithNativeVideoPublicationSettings:
    (const oms::base::VideoPublicationSettings &)nativeSettings {
  if (self = [super init]) {
    _nativeSettings = nativeSettings;
  }
  return self;
}

- (OMSVideoCodecParameters*)codec {
  return [[OMSVideoCodecParameters alloc]
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

@implementation OMSAudioSubscriptionCapabilities

@dynamic codecs;

- (instancetype)initWithNativeAudioSubscriptionCapabilities:
    (const oms::base::AudioSubscriptionCapabilities&)nativeCapabilities {
  if (self = [super init]) {
    _nativeCapabilities = nativeCapabilities;
  }
  return self;
}

- (NSArray<OMSAudioCodecParameters*>*)codecs {
  NSMutableArray<OMSAudioCodecParameters*>* codecArray =
      [NSMutableArray arrayWithCapacity:_nativeCapabilities.codecs.size()];
  for (auto& c : _nativeCapabilities.codecs) {
    OMSAudioCodecParameters* p =
        [[OMSAudioCodecParameters alloc] initWithNativeAudioCodecParameters:c];
    [codecArray addObject:p];
  }
  return codecArray;
}

@end

@implementation OMSVideoSubscriptionCapabilities

@dynamic codecs, resolutions, frameRates, bitrateMultipliers, keyframeIntervals;

- (instancetype)initWithNativeVideoSubscriptionCapabilities:
    (const oms::base::VideoSubscriptionCapabilities&)nativeCapabilities {
  if (self = [super init]) {
    _nativeCapabilities = nativeCapabilities;
  }
  return self;
}

- (NSArray<OMSVideoCodecParameters*>*)codecs {
  NSMutableArray<OMSVideoCodecParameters*>* codecArray =
      [NSMutableArray arrayWithCapacity:_nativeCapabilities.codecs.size()];
  for (const auto& c : _nativeCapabilities.codecs) {
    OMSVideoCodecParameters* p =
        [[OMSVideoCodecParameters alloc] initWithNativeVideoCodecParameters:c];
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
    NSNumber* v = [NSNumber numberWithDouble:f];
    [values addObject:v];
  }
  return values;
}

- (NSArray<NSNumber*>*)bitrateMultipliers {
  NSMutableArray<NSNumber*>* values = [NSMutableArray
      arrayWithCapacity:_nativeCapabilities.bitrate_multipliers.size()];
  for (const auto& f : _nativeCapabilities.bitrate_multipliers) {
    NSNumber* v = [NSNumber numberWithDouble:f];
    [values addObject:v];
  }
  return values;
}

- (NSArray<NSNumber*>*)keyframeIntervals {
  NSMutableArray<NSNumber*>* values = [NSMutableArray
      arrayWithCapacity:_nativeCapabilities.keyframe_intervals.size()];
  for (const auto& f : _nativeCapabilities.keyframe_intervals) {
    NSNumber* v = [NSNumber numberWithDouble:f];
    [values addObject:v];
  }
  return values;
}

@end


@implementation OMSSubscriptionCapabilities

- (instancetype)initWithNativeSubscriptionCapabilities:
    (const oms::base::SubscriptionCapabilities&)nativeCapabilities {
  if (self = [super init]) {
    _nativeCapabilities = nativeCapabilities;
    _audio = [[OMSAudioSubscriptionCapabilities alloc]
        initWithNativeAudioSubscriptionCapabilities:nativeCapabilities.audio];
    _video = [[OMSVideoSubscriptionCapabilities alloc]
        initWithNativeVideoSubscriptionCapabilities:nativeCapabilities.video];
  }
  return self;
}

@end

@implementation OMSStreamSourceInfo

@dynamic nativeStreamSourceInfo;

static std::unordered_map<OMSAudioSourceInfo,const oms::base::AudioSourceInfo>
    audioSourceInfoMap = {
        {OMSAudioSourceInfoMic, oms::base::AudioSourceInfo::kMic},
        {OMSAudioSourceInfoScreenCast, oms::base::AudioSourceInfo::kScreenCast},
        {OMSAudioSourceInfoFile, oms::base::AudioSourceInfo::kFile},
        {OMSAudioSourceInfoMixed, oms::base::AudioSourceInfo::kMixed},
        {OMSAudioSourceInfoUnknown, oms::base::AudioSourceInfo::kUnknown}};

static std::unordered_map<OMSVideoSourceInfo,const oms::base::VideoSourceInfo>
    videoSourceInfoMap = {
        {OMSVideoSourceInfoCamera, oms::base::VideoSourceInfo::kCamera},
        {OMSVideoSourceInfoScreenCast, oms::base::VideoSourceInfo::kScreenCast},
        {OMSVideoSourceInfoFile, oms::base::VideoSourceInfo::kFile},
        {OMSVideoSourceInfoMixed, oms::base::VideoSourceInfo::kMixed},
        {OMSVideoSourceInfoUnknown, oms::base::VideoSourceInfo::kUnknown}};

- (instancetype)initWithNativeStreamSourceInfo:
    (std::shared_ptr<oms::base::StreamSourceInfo>)nativeSource {
  if (self = [super init]) {
    auto it_audio =
        std::find_if(audioSourceInfoMap.begin(), audioSourceInfoMap.end(),
                     [&nativeSource](auto&& source) {
                       return source.second == nativeSource->audio;
                     });
    if (it_audio != audioSourceInfoMap.end()) {
      _audio = it_audio->first;
    } else {
      RTC_NOTREACHED();
      _audio = OMSAudioSourceInfoUnknown;
    }
    auto it_video =
        std::find_if(videoSourceInfoMap.begin(), videoSourceInfoMap.end(),
                     [&nativeSource](auto&& source) {
                       return source.second == nativeSource->video;
                     });
    if (it_video != videoSourceInfoMap.end()) {
      _video = it_video->first;
    } else {
      RTC_NOTREACHED();
      _video = OMSVideoSourceInfoUnknown;
    }
  }
  return self;
}

- (oms::base::StreamSourceInfo)nativeStreamSourceInfo {
  oms::base::AudioSourceInfo audio_source_info = audioSourceInfoMap[_audio];
  oms::base::VideoSourceInfo video_source_info = videoSourceInfoMap[_video];
  return oms::base::StreamSourceInfo(audio_source_info, video_source_info);
}

@end

@implementation OMSVideoTrackConstraints

@end

@implementation OMSStreamConstraints

@end

@implementation OMSAudioEncodingParameters

@dynamic nativeAudioEncodingParameters;

- (oms::base::AudioEncodingParameters)nativeAudioEncodingParameters {
  return oms::base::AudioEncodingParameters(
      *[self.codec nativeAudioCodecParameters].get(), self.maxBitrate);
}

@end

@implementation OMSVideoEncodingParameters

- (oms::base::VideoEncodingParameters)nativeVideoEncodingParameters {
  return oms::base::VideoEncodingParameters(
      *[self.codec nativeVideoCodecParameters].get(), self.maxBitrate, false);
}

@end

@implementation OMSTrackKindConverter

+ (oms::base::TrackKind)cppTrackKindForObjcTrackKind:(OMSTrackKind)kind {
  if (kind == (OMSTrackKindAudio | OMSTrackKindVideo)) {
    return oms::base::TrackKind::kAudioAndVideo;
  }
  if (kind == OMSTrackKindAudio) {
    return oms::base::TrackKind::kAudio;
  }
  if (kind == OMSTrackKindVideo) {
    return oms::base::TrackKind::kVideo;
  }
  if (kind != OMSTrackKindUnknown) {
    RTC_NOTREACHED();
  }
  return oms::base::TrackKind::kUnknown;
}

+ (OMSTrackKind)objcTrackKindForCppTrackKind:(oms::base::TrackKind)kind {
  switch (kind) {
    case oms::base::TrackKind::kAudioAndVideo:
      return OMSTrackKindAudio | OMSTrackKindVideo;
    case oms::base::TrackKind::kAudio:
      return OMSTrackKindAudio;
    case oms::base::TrackKind::kVideo:
      return OMSTrackKindVideo;
    case oms::base::TrackKind::kUnknown:
      return OMSTrackKindUnknown;
    default:
      RTC_NOTREACHED();
      return OMSTrackKindUnknown;
  }
}

@end
