//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <unordered_map>
#import "talk/ics/sdk/base/objc/RTCMediaCodec+Internal.h"
#import "webrtc/rtc_base/checks.h"

@implementation RTCMediaCodec

@end

@implementation RTCMediaCodec (Internal)

static std::unordered_map<NSInteger,
                          const ics::base::AudioCodec>
    audioCodecMap = {
        {AudioCodecOpus, ics::base::AudioCodec::kOPUS},
        {AudioCodecIsac, ics::base::AudioCodec::kISAC},
        {AudioCodecG722, ics::base::AudioCodec::kG722},
        {AudioCodecPcmu, ics::base::AudioCodec::kPCMU},
        {AudioCodecPcma, ics::base::AudioCodec::kPCMA}};

static std::unordered_map<NSInteger,
                          const ics::base::VideoCodec>
    videoCodecMap = {
        {VideoCodecVP8, ics::base::VideoCodec::kVP8},
        {VideoCodecH264, ics::base::VideoCodec::kH264}};

+ (ics::base::AudioCodec)nativeAudioCodec:
    (NSInteger)audioCodec {
  auto nativeAudioCodecIt = audioCodecMap.find(audioCodec);
  if (nativeAudioCodecIt == audioCodecMap.end()) {
    RTC_DCHECK(false);
    return ics::base::AudioCodec::kOPUS;
  }
  return nativeAudioCodecIt->second;
}

+ (ics::base::VideoCodec)nativeVideoCodec:
    (NSInteger)videoCodec {
  auto nativeVideoCodecIt = videoCodecMap.find(videoCodec);
  if (nativeVideoCodecIt == videoCodecMap.end()) {
    RTC_DCHECK(false);
    return ics::base::VideoCodec::kVP8;
  }
  return nativeVideoCodecIt->second;
}

@end
