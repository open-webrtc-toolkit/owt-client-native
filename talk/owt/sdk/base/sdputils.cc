// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include <regex>
#include <sstream>
#include <vector>
#include <unordered_map>
#include "talk/owt/sdk/base/sdputils.h"
#include "webrtc/rtc_base/logging.h"
using namespace rtc;
namespace owt {
namespace base {
static const std::unordered_map<AudioCodec, const std::string, EnumClassHash>
    audio_codec_names = {{AudioCodec::kOpus, "OPUS"},
                         {AudioCodec::kIsac, "ISAC"},
                         {AudioCodec::kG722, "G722"},
                         {AudioCodec::kPcmu, "PCMU"},
                         {AudioCodec::kIlbc, "ILBC"},
                         {AudioCodec::kPcma, "PCMA"}};
static const std::unordered_map<VideoCodec, const std::string, EnumClassHash>
    video_codec_names = {{VideoCodec::kVp8, "VP8"},
                         {VideoCodec::kH264, "H264"},
                         {VideoCodec::kVp9, "VP9"},
                         {VideoCodec::kH265, "H265"}};
std::string SdpUtils::SetPreferAudioCodecs(const std::string& original_sdp,
                                          std::vector<AudioCodec>& codec) {
  std::string cur_sdp(original_sdp);
  if (codec.size() == 0)
    return cur_sdp;
  std::vector<AudioCodec> rcodecs(codec.rbegin(), codec.rend());
  std::vector<std::string> codec_names;
  for (auto codec_current : rcodecs) {
    auto codec_it = audio_codec_names.find(codec_current);
    if (codec_it == audio_codec_names.end()) {
      RTC_LOG(LS_WARNING) << "Preferred audio codec is not available.";
      continue;
    }
    codec_names.push_back(codec_it->second);
  }
  cur_sdp = SdpUtils::SetPreferCodecs(cur_sdp, codec_names, true);
  return cur_sdp;
}
std::string SdpUtils::SetPreferVideoCodecs(const std::string& original_sdp,
                                          std::vector<VideoCodec>& codec) {
  std::string cur_sdp(original_sdp);
  if (codec.size() == 0)
    return cur_sdp;
  std::vector<VideoCodec> rcodecs(codec.rbegin(), codec.rend());
  std::vector<std::string> codec_names;
  for (auto codec_current : rcodecs) {
    auto codec_it = video_codec_names.find(codec_current);
    if (codec_it == video_codec_names.end()) {
      RTC_LOG(LS_WARNING) << "Preferred video codec is not available.";
      continue;
    }
    codec_names.push_back(codec_it->second);
  }
  cur_sdp = SdpUtils::SetPreferCodecs(cur_sdp, codec_names, false);
  return cur_sdp;
}

std::string SdpUtils::SetStartVideoBandwidth(const std::string& sdp,
                                             int bandwidth) {
  std::regex reg_red_codec("a=rtpmap:(\\d+) red\\/\\d+[\r]?[\n]?",
                           std::regex_constants::icase);
  std::smatch fmtp_map_match;
  std::string red_codec;
  auto search_result = std::regex_search(sdp, fmtp_map_match, reg_red_codec);
  if (search_result && fmtp_map_match.size() > 0) {
    red_codec = fmtp_map_match[1];
  }

  std::regex reg_video("m=video .*? ([0-9 ]+)[\r]?[\n]?",
                       std::regex_constants::icase);
  search_result = std::regex_search(sdp, fmtp_map_match, reg_video);
  if (!search_result && fmtp_map_match.size() == 0) {
    return sdp;
  }
  std::string str = fmtp_map_match[1];
  int start = 0;
  std::string delim = " ";
  size_t idx = str.find(delim, start);
  std::string ret_str = sdp;
  std::string reg_start_replace = "(a=fmtp:";
  std::string reg_end_replace = ")";
  std::string to_replace =
      "$1 x-google-start-bitrate=" + std::to_string(bandwidth);
  std::string reg_start_insert = "(a=rtpmap:(";
  std::string reg_end_insert = ") .*[\r]?[\n]?)";
  std::string to_insert =
      "$1a=fmtp:$2 x-google-start-bitrate=" + std::to_string(bandwidth) + "\n";
  std::regex reg_fmtp_video;
  std::string sub_str = "";
  while (idx != std::string::npos) {
    sub_str = str.substr(start, idx - start);
    start = idx + delim.size();
    idx = str.find(delim, start);
    if (sub_str.compare(red_codec) == 0) {
      continue;
    }
    reg_fmtp_video.assign(reg_start_replace + sub_str + reg_end_replace,
                          std::regex_constants::icase);
    search_result = std::regex_search(ret_str, fmtp_map_match, reg_fmtp_video);
    if (!search_result && fmtp_map_match.size() == 0) {
      reg_fmtp_video.assign(reg_start_insert + sub_str + reg_end_insert,
                            std::regex_constants::icase);
      ret_str = std::regex_replace(ret_str, reg_fmtp_video, to_insert);
    } else {
      ret_str = std::regex_replace(ret_str, reg_fmtp_video, to_replace);
    }
  }
  sub_str = str.substr(start);
  if (sub_str.compare(red_codec) == 0) {
    return ret_str;
  }
  reg_fmtp_video.assign(reg_start_replace + sub_str + reg_end_replace,
                        std::regex_constants::icase);
  search_result = std::regex_search(ret_str, fmtp_map_match, reg_fmtp_video);
  if (!search_result && fmtp_map_match.size() == 0) {
    reg_fmtp_video.assign(reg_start_insert + sub_str + reg_end_insert,
                          std::regex_constants::icase);
    ret_str = std::regex_replace(ret_str, reg_fmtp_video, to_insert);
  } else {
    ret_str = std::regex_replace(ret_str, reg_fmtp_video, to_replace);
  }
  return ret_str;
}

std::vector<std::string> SdpUtils::GetCodecValues(const std::string& sdp,
    std::string& codec_name,
    bool is_audio) {
  std::vector<std::string> codec_values;
  std::string sdp_current(sdp);
  std::regex reg_rtp_map(
      "a=rtpmap:(\\d+) " + codec_name + "\\/\\d+(?=[\r]?[\n]?)",
      std::regex_constants::icase);
  std::smatch rtp_map_match;
  while (std::regex_search(sdp_current, rtp_map_match, reg_rtp_map)) {
    codec_values.push_back(rtp_map_match[1]);
    sdp_current = rtp_map_match.suffix();
  }
  return codec_values;
}
// Remove non-prefer codecs out of the list. Keeping red and ulpfec,
// assuming the binding to original codec is out-of-bound.
// Keeping corresponding rtx payloads. Reorder m-line according to
// the reverse order of input codec names.
// TODO: unify to std::regex impl for Linux builds.
std::string SdpUtils::SetPreferCodecs(const std::string& sdp,
    std::vector<std::string>& codec_names,
    bool is_audio) {
  // Search all rtx maps in the sdp.
  std::regex reg_fmtp_apt(
      "a=fmtp:(\\d+) apt=(\\d+)(?=[\r]?[\n]?)",
      std::regex_constants::icase);
  std::smatch rtx_map_match;
  // Key is the rtx payload type, value is the original payload type.
  std::unordered_map<std::string, std::string> rtx_maps;
  std::string current_sdp = sdp;
  while (std::regex_search(current_sdp, rtx_map_match, reg_fmtp_apt)) {
    rtx_maps.insert({ rtx_map_match.str(1), rtx_map_match.str(2) });
    current_sdp = rtx_map_match.suffix();
  }
  std::vector<std::string> kept_codec_values;
  if (!is_audio) {
    // Get red and ulpfec payload type if any.
    bool has_red = false, has_ulpfec = false, has_flexfec = false;
    std::regex reg_red_map(
        "a=rtpmap:(\\d+) red\\/\\d+(?=[\r]?[\n]?)",
        std::regex_constants::icase);
    std::smatch red_map_match;
    std::string red_codec_value;
    auto search_result = std::regex_search(sdp, red_map_match, reg_red_map);
    if (search_result && red_map_match.size() != 0) {
      red_codec_value = red_map_match[1];
      has_red = true;
    }
    std::regex reg_ulpfec_map(
        "a=rtpmap:(\\d+) ulpfec\\/\\d+(?=[\r]?[\n]?)",
        std::regex_constants::icase);
    std::smatch ulpfec_map_match;
    std::string ulpfec_codec_value;
    search_result = std::regex_search(sdp, ulpfec_map_match, reg_ulpfec_map);
    if (search_result && ulpfec_map_match.size() != 0) {
      ulpfec_codec_value = ulpfec_map_match[1];
      has_ulpfec = true;
    }
    std::regex reg_flexfec_map("a=rtpmap:(\\d+) flexfec-03\\/\\d+(?=[\r]?[\n]?)",
                           std::regex_constants::icase);
    std::smatch flexfec_map_match;
    std::string flexfec_codec_value;
    search_result = std::regex_search(sdp, flexfec_map_match, reg_flexfec_map);
    if (search_result && flexfec_map_match.size() != 0) {
      flexfec_codec_value = flexfec_map_match[1];
      has_flexfec = true;
    }
    if (has_red) {
      kept_codec_values.push_back(red_codec_value);
      for (auto& rtx_value : rtx_maps) {
        if (rtx_value.second == red_codec_value) {
            kept_codec_values.push_back(rtx_value.first);
        }
      }
    }
    if (has_ulpfec) {
      kept_codec_values.push_back(ulpfec_codec_value);
      for (auto& rtx_value : rtx_maps) {
        if (rtx_value.second == ulpfec_codec_value) {
            kept_codec_values.push_back(rtx_value.first);
        }
      }
    }
    if (has_flexfec) {
      kept_codec_values.push_back(flexfec_codec_value);
      // flex-fec does not involve rtx so we don't search its rtx association.
    }
  }
  for (auto& codec_name : codec_names) {
    std::vector<std::string> codec_values = GetCodecValues(sdp, codec_name, is_audio);
    for (auto& value : codec_values) {
      // Input codec names are in reverse order, so the highest priortiy will be
      // placed at the beginning.
      kept_codec_values.insert(kept_codec_values.begin(), value);
      for (auto& rtx_value : rtx_maps) {
        if (rtx_value.second == value) {
          kept_codec_values.push_back(rtx_value.first);
        }
      }
    }
  }
  std::string media_type;
  media_type = is_audio ? "audio" : "video";
  std::regex reg_m_line("m=" + media_type + ".*(?=[\r]?[\n]?)");
  std::smatch m_line_match;
  auto search_result = std::regex_search(sdp, m_line_match, reg_m_line);
  if (!search_result || m_line_match.size() == 0) {
    RTC_LOG(LS_WARNING) << "M-line is not found. SDP: " << sdp;
    return sdp;
  }
  std::string m_line(m_line_match[0]);
  // Split m_line into vector and put preferred codec in the first place.
  std::vector<std::string> m_line_vector;
  std::stringstream original_m_line_stream(m_line);
  std::string item;
  while (std::getline(original_m_line_stream, item, ' ')) {
    m_line_vector.push_back(item);
  }
  if (m_line_vector.size() < 3) {
    RTC_LOG(LS_WARNING) << "Wrong SDP format description: " << m_line;
    return sdp;
  }
  std::stringstream m_line_stream;
  for (int i = 0; i < 3; i++) {
    if (i < 2)
      m_line_stream << m_line_vector[i] << " ";
    else
      m_line_stream << m_line_vector[i];
  }
  for (auto& codec_value : kept_codec_values) {
    m_line_stream << " " << codec_value;
  }
  RTC_LOG(LS_INFO) << "New m-line: " << m_line_stream.str();
  std::string before_strip = std::regex_replace(sdp, reg_m_line, m_line_stream.str());
  std::string after_strip = before_strip;
  // Remove all a=fmtp:xx, a=rtpmap:xx and a=rtcp-fb:xx where xx is not in m-line,
  // this includes the a=fmtp:xx apt:yy lines for rtx.
  for (size_t i = 3, m_line_vector_size = m_line_vector.size(); i < m_line_vector_size; i++) {
    if (std::find(kept_codec_values.begin(), kept_codec_values.end(),
      m_line_vector[i]) == kept_codec_values.end()) {
      std::string codec_value = m_line_vector[i];
      std::regex reg_rtp_xx_map(
          "a=rtpmap:" + codec_value + " .*\\r\\n",
          std::regex_constants::icase);
      after_strip = std::regex_replace(before_strip, reg_rtp_xx_map, "");
      before_strip = after_strip;
      std::regex reg_fmtp_xx_map(
          "a=fmtp:" + codec_value + " .*\\r\\n",
          std::regex_constants::icase);
      after_strip = std::regex_replace(before_strip, reg_fmtp_xx_map, "");
      before_strip = after_strip;
      std::regex reg_rtcp_map(
          "a=rtcp-fb:" + codec_value + " .*\\r\\n",
          std::regex_constants::icase);
      after_strip = std::regex_replace(before_strip, reg_rtcp_map, "");
      before_strip = after_strip;
    }
  }
  return after_strip;
}
}
}
