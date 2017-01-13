//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <unordered_map>
#import "talk/woogeen/sdk/base/objc/RTCMediaCodec+Internal.h"
#import "webrtc/base/checks.h"

@implementation RTCMediaCodec

@end

@implementation RTCMediaCodec (Internal)

static std::unordered_map<NSInteger,
                          const woogeen::base::MediaCodec::AudioCodec>
    audioCodecMap = {
        {AudioCodecOpus, woogeen::base::MediaCodec::AudioCodec::OPUS},
        {AudioCodecIsac, woogeen::base::MediaCodec::AudioCodec::ISAC},
        {AudioCodecG722, woogeen::base::MediaCodec::AudioCodec::G722},
        {AudioCodecPcmu, woogeen::base::MediaCodec::AudioCodec::PCMU},
        {AudioCodecPcma, woogeen::base::MediaCodec::AudioCodec::PCMA}};

static std::unordered_map<NSInteger,
                          const woogeen::base::MediaCodec::VideoCodec>
    videoCodecMap = {
        {VideoCodecVP8, woogeen::base::MediaCodec::VideoCodec::VP8},
        {VideoCodecH264, woogeen::base::MediaCodec::VideoCodec::H264}};

+ (woogeen::base::MediaCodec::AudioCodec)nativeAudioCodec:
    (NSInteger)audioCodec {
  auto nativeAudioCodecIt = audioCodecMap.find(audioCodec);
  if (nativeAudioCodecIt == audioCodecMap.end()) {
    RTC_DCHECK(false);
    return woogeen::base::MediaCodec::AudioCodec::OPUS;
  }
  return nativeAudioCodecIt->second;
}

+ (woogeen::base::MediaCodec::VideoCodec)nativeVideoCodec:
    (NSInteger)videoCodec {
  auto nativeVideoCodecIt = videoCodecMap.find(videoCodec);
  if (nativeVideoCodecIt == videoCodecMap.end()) {
    RTC_DCHECK(false);
    return woogeen::base::MediaCodec::VideoCodec::VP8;
  }
  return nativeVideoCodecIt->second;
}

@end
