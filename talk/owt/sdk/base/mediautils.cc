// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include <algorithm>
#include <map>
#include <string>
#include "absl/types/optional.h"
#include "webrtc/common_video/h264/h264_common.h"
#include "webrtc/common_video/h264/prefix_parser.h"
#include "webrtc/rtc_base/checks.h"
#include "webrtc/rtc_base/bit_buffer.h"
#include "system_wrappers/include/field_trial.h"
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
absl::optional<unsigned int> MediaUtils::GetH264TemporalLayers() {
  std::string experiment_value =
      webrtc::field_trial::FindFullName("OWT-H264TemporalLayers");
  unsigned int layers = 1;
  if (sscanf(experiment_value.c_str(), "%u", &layers) != 1)
    return absl::nullopt;
  layers = std::min(layers, 4u);
  layers = std::max(layers, 1u);
  return layers;
}
bool ParseSlice(const uint8_t* slice, size_t length, int& temporal_id, int& priority_id, bool& is_idr) {
  using namespace webrtc;
  webrtc::H264::NaluType nalu_type = webrtc::H264::ParseNaluType(slice[0]);
  absl::optional<PrefixParser::PrefixState> prefix_state;
  switch (nalu_type) {
    case H264::NaluType::kPrefix: {
      prefix_state = PrefixParser::ParsePrefix(
          slice + webrtc::H264::kNaluTypeSize,
                                 length - webrtc::H264::kNaluTypeSize);
      break;
    }
    // Ignore all other cases as we only care about prefix NAL.
    default:
      break;
  }
  if (prefix_state.has_value()) {
    temporal_id = prefix_state.value().temporal_id;
    is_idr = prefix_state.value().idr_flag == 1 ? true : false;
    priority_id = prefix_state.value().priority_id;
    return true;
  }

  return false;
}
bool MediaUtils::GetH264TemporalInfo(uint8_t* buffer, size_t buffer_length,
  int& temporal_id, int& priority_id, bool& is_idr) {
  bool prefix_nal_found = false;
  std::vector<webrtc::H264::NaluIndex> nalu_indices =
      webrtc::H264::FindNaluIndices(buffer, buffer_length);
  for (const webrtc::H264::NaluIndex& index : nalu_indices) {
    prefix_nal_found = ParseSlice(&buffer[index.payload_start_offset],
      index.payload_size, temporal_id, priority_id, is_idr);
    if (prefix_nal_found)
      return true;
  }
  return prefix_nal_found;
}
}  // namespace base
}  // namespace owt
