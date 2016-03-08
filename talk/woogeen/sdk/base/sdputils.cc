/*
 * Intel License
 */

#include <regex>
#include <sstream>
#include <vector>
#include <unordered_map>
#include "talk/woogeen/sdk/base/sdputils.h"
#include "webrtc/base/logging.h"

namespace woogeen {
namespace base {

static const std::unordered_map<MediaCodec::AudioCodec,
                                const std::string,
                                std::hash<int>> audio_codec_names = {
    {MediaCodec::AudioCodec::OPUS, "OPUS"},
    {MediaCodec::AudioCodec::ISAC, "ISAC"},
    {MediaCodec::AudioCodec::G722, "G722"},
    {MediaCodec::AudioCodec::PCMU, "PCMU"},
    {MediaCodec::AudioCodec::PCMA, "PCMA"}};
static const std::unordered_map<MediaCodec::VideoCodec,
                                const std::string,
                                std::hash<int>> video_codec_names = {
    {MediaCodec::VideoCodec::VP8, "Vp8"},
    {MediaCodec::VideoCodec::H264, "H264"}};

std::string SdpUtils::SetMaximumVideoBandwidth(const std::string& sdp,
                                               int bitrate) {
  std::regex reg("a=mid:video\r\n");
  std::stringstream sdp_line_width_bitrate;
  sdp_line_width_bitrate << "a=mid:video\r\nb=AS: " << bitrate << "\r\n";
  return std::regex_replace(sdp, reg, sdp_line_width_bitrate.str());
}

std::string SdpUtils::SetMaximumAudioBandwidth(const std::string& sdp,
                                               int bitrate) {
  std::regex reg("a=mid:audio\r\n");
  std::stringstream sdp_line_width_bitrate;
  sdp_line_width_bitrate << "a=mid:audio\r\nb=AS: " << bitrate << "\r\n";
  return std::regex_replace(sdp, reg, sdp_line_width_bitrate.str());
}

std::string SdpUtils::SetPreferAudioCodec(const std::string& original_sdp,
                                          MediaCodec::AudioCodec codec) {
  auto codec_it = audio_codec_names.find(codec);
  if (codec_it == audio_codec_names.end()) {
    LOG(LS_WARNING) << "Preferred audio codec is not available.";
    return original_sdp;
  }
  return SdpUtils::SetPreferCodec(original_sdp, codec_it->second, true);
}

std::string SdpUtils::SetPreferVideoCodec(const std::string& original_sdp,
                                          MediaCodec::VideoCodec codec) {
  auto codec_it = video_codec_names.find(codec);
  if (codec_it == video_codec_names.end()) {
    LOG(LS_WARNING) << "Preferred video codec is not available.";
    return original_sdp;
  }
  return SdpUtils::SetPreferCodec(original_sdp, codec_it->second, false);
}

std::string SdpUtils::SetPreferCodec(const std::string& sdp,
                                     const std::string& codec_name,
                                     bool is_audio) {
  std::string media_type;
  media_type = is_audio ? "audio" : "video";
  std::regex reg_m_line("m=" + media_type + ".*(?=[\r]?[\n]?)");
  std::regex reg_rtp_map(
      "a=rtpmap:(\\d+) " + codec_name + "\\/\\d+(?=[\r]?[\n]?)",
      std::regex_constants::icase);
  std::smatch rtp_map_match;
  auto search_result = std::regex_search(sdp, rtp_map_match, reg_rtp_map);
  if (!search_result || rtp_map_match.size() == 0) {
    LOG(LS_WARNING) << "RTP map for " << codec_name
                    << " is not found. SDP: " << sdp;
    return sdp;
  }
  LOG(LS_INFO) << "RTP map found for " << codec_name << ": "
               << rtp_map_match[0];
  std::string codec_value(rtp_map_match[1]);
  std::smatch m_line_match;
  search_result = std::regex_search(sdp, m_line_match, reg_m_line);
  if (!search_result || m_line_match.size() == 0) {
    LOG(LS_WARNING) << "M-line is not found. SDP: " << sdp;
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
    LOG(LS_WARNING) << "Wrong SDP format description: " << m_line;
    return sdp;
  }
  std::stringstream m_line_stream;
  for (int i = 0; i < 3; i++) {
    m_line_stream << m_line_vector[i] << " ";
  }
  m_line_stream << codec_value;
  for (int i = 3, m_line_vector_size = m_line_vector.size();
       i < m_line_vector_size; i++) {
    if (m_line_vector[i] != codec_value) {
      m_line_stream << " " << m_line_vector[i];
    }
  }
  LOG(LS_INFO) << "New m-line: " << m_line_stream.str();
  return std::regex_replace(sdp, reg_m_line, m_line_stream.str());
}
}
}
