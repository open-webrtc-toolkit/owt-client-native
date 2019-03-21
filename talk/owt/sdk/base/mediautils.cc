// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include <algorithm>
#include <map>
#include <string>
#include "webrtc/rtc_base/checks.h"
#include "talk/owt/sdk/base/mediautils.h"
namespace owt {
namespace base {
static const std::map<const std::string, const Resolution> resolution_name_map = {
    {"cif", Resolution(352, 288)},
    {"vga", Resolution(640, 480)},
    {"hd720p", Resolution(1280, 720)},
    {"hd1080p", Resolution(1920, 1080)}};
static const std::map<const std::string, const AudioCodec>
    audio_codec_names = {
        {"opus", AudioCodec::kOpus}, {"isac", AudioCodec::kIsac},
        {"g722", AudioCodec::kG722}, {"pcmu", AudioCodec::kPcmu},
        {"pcma", AudioCodec::kPcma}, {"ilbc", AudioCodec::kIlbc},
        {"aac", AudioCodec::kAac},   {"ac3", AudioCodec::kAc3},
        {"asao", AudioCodec::kAsao}, {"unknown", AudioCodec::kUnknown}};
static const std::map<const std::string, const VideoCodec>
    video_codec_names = {{"vp8", VideoCodec::kVp8},
                         {"vp9", VideoCodec::kVp9},
                         {"h264", VideoCodec::kH264},
                         {"h265", VideoCodec::kH265}};
std::string MediaUtils::GetResolutionName(const Resolution& resolution) {
  for (auto it = resolution_name_map.begin(); it != resolution_name_map.end();
       ++it) {
    if (it->second == resolution) {
      return it->first;
    }
  }
  return "";
}
AudioCodec MediaUtils::GetAudioCodecFromString(const std::string& codec_name) {
  auto it = audio_codec_names.find(codec_name);
  if (it != audio_codec_names.end()) {
    return it->second;
  }
  RTC_NOTREACHED();
  return AudioCodec::kUnknown;
}
VideoCodec MediaUtils::GetVideoCodecFromString(const std::string& codec_name) {
  auto it = video_codec_names.find(codec_name);
  if (it != video_codec_names.end()) {
    return it->second;
  }
  RTC_NOTREACHED();
  return VideoCodec::kUnknown;
}
std::string MediaUtils::AudioCodecToString(const AudioCodec& audio_codec) {
  auto it = std::find_if(audio_codec_names.begin(), audio_codec_names.end(),
                         [&audio_codec](const auto& codec) {
                           return codec.second == audio_codec;
                         });
  if (it != audio_codec_names.end()) {
    return it->first;
  } else {
    RTC_NOTREACHED();
    return "unknown";
  }
}
std::string MediaUtils::VideoCodecToString(const VideoCodec& video_codec) {
  auto it = std::find_if(video_codec_names.begin(), video_codec_names.end(),
                         [&video_codec](const auto& codec) {
                           return codec.second == video_codec;
                         });
  if (it != video_codec_names.end()) {
    return it->first;
  } else {
    RTC_NOTREACHED();
    return "unknown";
  }
}
}  // namespace base
}  // namespace owt
