//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "talk/ics/sdk/base/objc/ICSMediaFormat+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"

#include "talk/ics/sdk/include/cpp/ics/base/options.h"
#include "webrtc/rtc_base/checks.h"

@implementation ICSAudioCodecParameters {
  std::shared_ptr<ics::base::AudioCodecParameters> _nativeAudioCodecParameters;
}

@dynamic nativeAudioCodecParameters;

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
    auto it =
        std::find_if(audioCodecMap.begin(), audioCodecMap.end(),
                     [&nativeAudioCodecParameters](auto&& codec) {
                       return codec.second == nativeAudioCodecParameters.name;
                     });
    if (it != audioCodecMap.end()) {
      _name = it->first;
    } else {
      RTC_NOTREACHED();
      _name = ICSAudioCodecUnknown;
    }
    _channelCount = nativeAudioCodecParameters.channel_count;
    _clockRate = nativeAudioCodecParameters.clock_rate;
  }
  return self;
}

- (std::shared_ptr<ics::base::AudioCodecParameters>)nativeAudioCodecParameters {
  ics::base::AudioCodec codec_name = audioCodecMap[self.name];
  auto nativeParameters = std::shared_ptr<ics::base::AudioCodecParameters>(
      new ics::base::AudioCodecParameters(codec_name, self.channelCount,
                                          self.clockRate));
  return nativeParameters;
}

@end

@implementation ICSVideoCodecParameters{
  std::shared_ptr<ics::base::VideoCodecParameters> _nativeVideoCodecParameters;
}

@dynamic name, profile, nativeVideoCodecParameters;

static std::unordered_map<ICSVideoCodec, const ics::base::VideoCodec>
    videoCodecMap = {{ICSVideoCodecVP8, ics::base::VideoCodec::kVp8},
                     {ICSVideoCodecVP9, ics::base::VideoCodec::kVp9},
                     {ICSVideoCodecH264, ics::base::VideoCodec::kH264},
                     {ICSVideoCodecH265, ics::base::VideoCodec::kH265}};

- (instancetype)initWithNativeVideoCodecParameters:
    (const ics::base::VideoCodecParameters&)nativeVideoCodecParameters {
  if (self = [super init]) {
    _nativeVideoCodecParameters =
        std::shared_ptr<ics::base::VideoCodecParameters>(
            new ics::base::VideoCodecParameters(nativeVideoCodecParameters));
  }
  return self;
}

- (ICSVideoCodec)name {
  auto it = std::find_if(
      videoCodecMap.begin(), videoCodecMap.end(), [self](auto&& codec) {
        return codec.second == _nativeVideoCodecParameters->name;
      });
  if (it != videoCodecMap.end()) {
    return it->first;
  } else {
    RTC_NOTREACHED();
    return ICSVideoCodecUnknown;
  }
}

- (std::shared_ptr<ics::base::VideoCodecParameters>)nativeVideoCodecParameters {
  if (_nativeVideoCodecParameters) {
    return _nativeVideoCodecParameters;
  }
  ics::base::VideoCodec codec_name = videoCodecMap[self.name];
  _nativeVideoCodecParameters =
      std::shared_ptr<ics::base::VideoCodecParameters>(
          new ics::base::VideoCodecParameters(
              codec_name, [NSString stdStringForString:self.profile]));
  return _nativeVideoCodecParameters;
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

@implementation ICSStreamSourceInfo

@dynamic nativeStreamSourceInfo;

static std::unordered_map<ICSAudioSourceInfo,const ics::base::AudioSourceInfo>
    audioSourceInfoMap = {
        {ICSAudioSourceInfoMic, ics::base::AudioSourceInfo::kMic},
        {ICSAudioSourceInfoScreenCast, ics::base::AudioSourceInfo::kScreenCast},
        {ICSAudioSourceInfoFile, ics::base::AudioSourceInfo::kFile},
        {ICSAudioSourceInfoMixed, ics::base::AudioSourceInfo::kMixed},
        {ICSAudioSourceInfoUnknown, ics::base::AudioSourceInfo::kUnknown}};

static std::unordered_map<ICSVideoSourceInfo,const ics::base::VideoSourceInfo>
    videoSourceInfoMap = {
        {ICSVideoSourceInfoCamera, ics::base::VideoSourceInfo::kCamera},
        {ICSVideoSourceInfoScreenCast, ics::base::VideoSourceInfo::kScreenCast},
        {ICSVideoSourceInfoFile, ics::base::VideoSourceInfo::kFile},
        {ICSVideoSourceInfoMixed, ics::base::VideoSourceInfo::kMixed},
        {ICSVideoSourceInfoUnknown, ics::base::VideoSourceInfo::kUnknown}};

- (instancetype)initWithNativeStreamSourceInfo:
    (std::shared_ptr<ics::base::StreamSourceInfo>)nativeSource {
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
      _audio = ICSAudioSourceInfoUnknown;
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
      _video = ICSVideoSourceInfoUnknown;
    }
  }
  return self;
}

- (ics::base::StreamSourceInfo)nativeStreamSourceInfo {
  ics::base::AudioSourceInfo audio_source_info = audioSourceInfoMap[_audio];
  ics::base::VideoSourceInfo video_source_info = videoSourceInfoMap[_video];
  return ics::base::StreamSourceInfo(audio_source_info, video_source_info);
}

@end

@implementation ICSVideoTrackConstraints

@end

@implementation ICSStreamConstraints

@end

@implementation ICSAudioEncodingParameters

@dynamic nativeAudioEncodingParameters;

- (ics::base::AudioEncodingParameters)nativeAudioEncodingParameters {
  return ics::base::AudioEncodingParameters(
      *[self.codec nativeAudioCodecParameters].get(), self.maxBitrate);
}

@end

@implementation ICSVideoEncodingParameters

@end

@implementation ICSTrackKindConverter

+ (ics::base::TrackKind)cppTrackKindForObjcTrackKind:(ICSTrackKind)kind {
  if (kind == (ICSTrackKindAudio & ICSTrackKindVideo)) {
    return ics::base::TrackKind::kAudioAndVideo;
  }
  if (kind == ICSTrackKindAudio) {
    return ics::base::TrackKind::kAudio;
  }
  if (kind == ICSTrackKindVideo) {
    return ics::base::TrackKind::kVideo;
  }
  if (kind != ICSTrackKindUnknown) {
    RTC_NOTREACHED();
  }
  return ics::base::TrackKind::kUnknown;
}

+ (ICSTrackKind)objcTrackKindForCppTrackKind:(ics::base::TrackKind)kind {
  switch (kind) {
    case ics::base::TrackKind::kAudioAndVideo:
      return ICSTrackKindAudio & ICSTrackKindVideo;
    case ics::base::TrackKind::kAudio:
      return ICSTrackKindAudio;
    case ics::base::TrackKind::kVideo:
      return ICSTrackKindVideo;
    case ics::base::TrackKind::kUnknown:
      return ICSTrackKindUnknown;
    default:
      RTC_NOTREACHED();
      return ICSTrackKindUnknown;
  }
}

@end
