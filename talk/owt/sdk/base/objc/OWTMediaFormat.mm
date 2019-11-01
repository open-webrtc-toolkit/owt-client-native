// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <UIKit/UIKit.h>
#import "talk/owt/sdk/base/objc/OWTMediaFormat+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
#include "talk/owt/sdk/include/cpp/owt/base/options.h"
#include "webrtc/rtc_base/checks.h"
@implementation OWTAudioCodecParameters
@dynamic nativeAudioCodecParameters;
static std::unordered_map<OWTAudioCodec, const owt::base::AudioCodec>
    audioCodecMap = {{OWTAudioCodecOpus, owt::base::AudioCodec::kOpus},
                     {OWTAudioCodecIsac, owt::base::AudioCodec::kIsac},
                     {OWTAudioCodecG722, owt::base::AudioCodec::kG722},
                     {OWTAudioCodecPcmu, owt::base::AudioCodec::kPcmu},
                     {OWTAudioCodecPcma, owt::base::AudioCodec::kPcma},
                     {OWTAudioCodecIlbc, owt::base::AudioCodec::kIlbc},
                     {OWTAudioCodecAac, owt::base::AudioCodec::kAac},
                     {OWTAudioCodecAc3, owt::base::AudioCodec::kAc3},
                     {OWTAudioCodecAsao, owt::base::AudioCodec::kAsao},
                     {OWTAudioCodecUnknown, owt::base::AudioCodec::kUnknown}};
- (instancetype)initWithNativeAudioCodecParameters:
    (const owt::base::AudioCodecParameters&)nativeAudioCodecParameters {
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
      _name = OWTAudioCodecUnknown;
    }
    _channelCount = nativeAudioCodecParameters.channel_count;
    _clockRate = nativeAudioCodecParameters.clock_rate;
  }
  return self;
}
- (std::shared_ptr<owt::base::AudioCodecParameters>)nativeAudioCodecParameters {
  owt::base::AudioCodec codec_name = audioCodecMap[self.name];
  auto nativeParameters = std::shared_ptr<owt::base::AudioCodecParameters>(
      new owt::base::AudioCodecParameters(codec_name, self.channelCount,
                                          self.clockRate));
  return nativeParameters;
}
@end
@implementation OWTVideoCodecParameters
@dynamic nativeVideoCodecParameters;
static std::unordered_map<OWTVideoCodec, const owt::base::VideoCodec>
    videoCodecMap = {{OWTVideoCodecVP8, owt::base::VideoCodec::kVp8},
                     {OWTVideoCodecVP9, owt::base::VideoCodec::kVp9},
                     {OWTVideoCodecH264, owt::base::VideoCodec::kH264},
                     {OWTVideoCodecH265, owt::base::VideoCodec::kH265}};
- (instancetype)initWithNativeVideoCodecParameters:
    (const owt::base::VideoCodecParameters&)nativeVideoCodecParameters {
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
      _name = OWTVideoCodecUnknown;
    }
    _profile =
        [NSString stringForStdString:nativeVideoCodecParameters.profile];
  }
  return self;
}
- (std::shared_ptr<owt::base::VideoCodecParameters>)nativeVideoCodecParameters {
  owt::base::VideoCodec codec_name = videoCodecMap[self.name];
  auto nativeVideoCodecParameters =
      std::shared_ptr<owt::base::VideoCodecParameters>(
          new owt::base::VideoCodecParameters(
              codec_name, [NSString stdStringForString:self.profile]));
  return nativeVideoCodecParameters;
}
@end
@implementation OWTPublicationSettings
- (instancetype)initWithNativePublicationSettings:
    (const owt::base::PublicationSettings &)nativeSettings {
  if (self = [super init]) {
    _nativeSettings = nativeSettings;
  }
  NSMutableArray<OWTAudioPublicationSettings*>* audio =
      [NSMutableArray arrayWithCapacity:nativeSettings.audio.size()];
  for (auto& nativeAudioSettings : _nativeSettings.audio) {
    OWTAudioPublicationSettings* objcAudioSettings =
        [[OWTAudioPublicationSettings alloc]
            initWithNativeAudioPublicationSettings:nativeAudioSettings];
    [audio addObject:objcAudioSettings];
  }
  _audio = audio;
  NSMutableArray<OWTVideoPublicationSettings*>* video =
      [NSMutableArray arrayWithCapacity:nativeSettings.video.size()];
  for (auto& nativeVideoSettings : _nativeSettings.video) {
    OWTVideoPublicationSettings* objcVideoSettings =
        [[OWTVideoPublicationSettings alloc]
            initWithNativeVideoPublicationSettings:nativeVideoSettings];
    [video addObject:objcVideoSettings];
  }
  _video = video;
  return self;
}
@end
@implementation OWTAudioPublicationSettings
@dynamic codec;
- (instancetype)initWithNativeAudioPublicationSettings:
    (const owt::base::AudioPublicationSettings &)nativeSettings {
  if (self = [super init]) {
    _nativeSettings = nativeSettings;
  }
  return self;
}
- (OWTAudioCodecParameters*)codec {
  return [[OWTAudioCodecParameters alloc]
      initWithNativeAudioCodecParameters:_nativeSettings.codec];
}
@end
@implementation OWTVideoPublicationSettings
@dynamic codec, resolution, frameRate, bitrate, keyframeInterval, rid;
- (instancetype)initWithNativeVideoPublicationSettings:
    (const owt::base::VideoPublicationSettings &)nativeSettings {
  if (self = [super init]) {
    _nativeSettings = nativeSettings;
  }
  return self;
}
- (OWTVideoCodecParameters*)codec {
  return [[OWTVideoCodecParameters alloc]
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
- (NSString*)rid {
  return [NSString stringForStdString:_nativeSettings.rid];
}
@end

@implementation OWTAudioSubscriptionCapabilities
@dynamic codecs;
- (instancetype)initWithNativeAudioSubscriptionCapabilities:
    (const owt::base::AudioSubscriptionCapabilities&)nativeCapabilities {
  if (self = [super init]) {
    _nativeCapabilities = nativeCapabilities;
  }
  return self;
}
- (NSArray<OWTAudioCodecParameters*>*)codecs {
  NSMutableArray<OWTAudioCodecParameters*>* codecArray =
      [NSMutableArray arrayWithCapacity:_nativeCapabilities.codecs.size()];
  for (auto& c : _nativeCapabilities.codecs) {
    OWTAudioCodecParameters* p =
        [[OWTAudioCodecParameters alloc] initWithNativeAudioCodecParameters:c];
    [codecArray addObject:p];
  }
  return codecArray;
}
@end
@implementation OWTVideoSubscriptionCapabilities
@dynamic codecs, resolutions, frameRates, bitrateMultipliers, keyframeIntervals;
- (instancetype)initWithNativeVideoSubscriptionCapabilities:
    (const owt::base::VideoSubscriptionCapabilities&)nativeCapabilities {
  if (self = [super init]) {
    _nativeCapabilities = nativeCapabilities;
  }
  return self;
}
- (NSArray<OWTVideoCodecParameters*>*)codecs {
  NSMutableArray<OWTVideoCodecParameters*>* codecArray =
      [NSMutableArray arrayWithCapacity:_nativeCapabilities.codecs.size()];
  for (const auto& c : _nativeCapabilities.codecs) {
    OWTVideoCodecParameters* p =
        [[OWTVideoCodecParameters alloc] initWithNativeVideoCodecParameters:c];
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

@implementation OWTSubscriptionCapabilities
- (instancetype)initWithNativeSubscriptionCapabilities:
    (const owt::base::SubscriptionCapabilities&)nativeCapabilities {
  if (self = [super init]) {
    _nativeCapabilities = nativeCapabilities;
    _audio = [[OWTAudioSubscriptionCapabilities alloc]
        initWithNativeAudioSubscriptionCapabilities:nativeCapabilities.audio];
    _video = [[OWTVideoSubscriptionCapabilities alloc]
        initWithNativeVideoSubscriptionCapabilities:nativeCapabilities.video];
  }
  return self;
}
@end
@implementation OWTStreamSourceInfo
@dynamic nativeStreamSourceInfo;
static std::unordered_map<OWTAudioSourceInfo,const owt::base::AudioSourceInfo>
    audioSourceInfoMap = {
        {OWTAudioSourceInfoMic, owt::base::AudioSourceInfo::kMic},
        {OWTAudioSourceInfoScreenCast, owt::base::AudioSourceInfo::kScreenCast},
        {OWTAudioSourceInfoFile, owt::base::AudioSourceInfo::kFile},
        {OWTAudioSourceInfoMixed, owt::base::AudioSourceInfo::kMixed},
        {OWTAudioSourceInfoUnknown, owt::base::AudioSourceInfo::kUnknown}};
static std::unordered_map<OWTVideoSourceInfo,const owt::base::VideoSourceInfo>
    videoSourceInfoMap = {
        {OWTVideoSourceInfoCamera, owt::base::VideoSourceInfo::kCamera},
        {OWTVideoSourceInfoScreenCast, owt::base::VideoSourceInfo::kScreenCast},
        {OWTVideoSourceInfoFile, owt::base::VideoSourceInfo::kFile},
        {OWTVideoSourceInfoMixed, owt::base::VideoSourceInfo::kMixed},
        {OWTVideoSourceInfoUnknown, owt::base::VideoSourceInfo::kUnknown}};
- (instancetype)initWithNativeStreamSourceInfo:
    (std::shared_ptr<owt::base::StreamSourceInfo>)nativeSource {
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
      _audio = OWTAudioSourceInfoUnknown;
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
      _video = OWTVideoSourceInfoUnknown;
    }
  }
  return self;
}
- (owt::base::StreamSourceInfo)nativeStreamSourceInfo {
  owt::base::AudioSourceInfo audio_source_info = audioSourceInfoMap[_audio];
  owt::base::VideoSourceInfo video_source_info = videoSourceInfoMap[_video];
  return owt::base::StreamSourceInfo(audio_source_info, video_source_info);
}
@end
@implementation OWTVideoTrackConstraints
@end
@implementation OWTStreamConstraints
@end
@implementation OWTAudioEncodingParameters
@dynamic nativeAudioEncodingParameters;
- (owt::base::AudioEncodingParameters)nativeAudioEncodingParameters {
  return owt::base::AudioEncodingParameters(
      *[self.codec nativeAudioCodecParameters].get(), self.maxBitrate);
}
@end
@implementation OWTVideoEncodingParameters
- (owt::base::VideoEncodingParameters)nativeVideoEncodingParameters {
  return owt::base::VideoEncodingParameters(
      *[self.codec nativeVideoCodecParameters].get(), self.maxBitrate, false);
}
@end
@implementation OWTTrackKindConverter
+ (owt::base::TrackKind)cppTrackKindForObjcTrackKind:(OWTTrackKind)kind {
  if (kind == (OWTTrackKindAudio | OWTTrackKindVideo)) {
    return owt::base::TrackKind::kAudioAndVideo;
  }
  if (kind == OWTTrackKindAudio) {
    return owt::base::TrackKind::kAudio;
  }
  if (kind == OWTTrackKindVideo) {
    return owt::base::TrackKind::kVideo;
  }
  if (kind != OWTTrackKindUnknown) {
    RTC_NOTREACHED();
  }
  return owt::base::TrackKind::kUnknown;
}
+ (OWTTrackKind)objcTrackKindForCppTrackKind:(owt::base::TrackKind)kind {
  switch (kind) {
    case owt::base::TrackKind::kAudioAndVideo:
      return OWTTrackKindAudio | OWTTrackKindVideo;
    case owt::base::TrackKind::kAudio:
      return OWTTrackKindAudio;
    case owt::base::TrackKind::kVideo:
      return OWTTrackKindVideo;
    case owt::base::TrackKind::kUnknown:
      return OWTTrackKindUnknown;
    default:
      RTC_NOTREACHED();
      return OWTTrackKindUnknown;
  }
}
@end
