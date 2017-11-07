/*
 * Intel License
 */

#include <map>
#include <string>
#include "talk/woogeen/sdk/base/mediautils.h"
#include "webrtc/base/checks.h"

namespace woogeen {
namespace base {
  static std::map<const std::string, const Resolution>
    resolution_name_map = {
        {"cif", Resolution(352, 288)},
        {"vga", Resolution(640, 480)},
        {"hd720p", Resolution(1280, 720)},
        {"hd1080p", Resolution(1920, 1080)}};

  std::string MediaUtils::GetResolutionName(const Resolution& resolution) {
    for(auto it=resolution_name_map.begin();it!=resolution_name_map.end();++it){
      if(it->second==resolution){
        return it->first;
      }
    }
    return "";
  }

  std::string MediaUtils::AudioCodecToString(
      const MediaCodec::AudioCodec& audio_codec) {
    switch (audio_codec) {
      case MediaCodec::AudioCodec::OPUS:
        return "opus";
      case MediaCodec::AudioCodec::ISAC:
        return "isac";
      case MediaCodec::AudioCodec::G722:
        return "g722";
      case MediaCodec::AudioCodec::PCMU:
        return "pcmu";
      case MediaCodec::AudioCodec::PCMA:
        return "pcma";
      default:
        RTC_NOTREACHED();
        return "";
    }
  }

  std::string MediaUtils::VideoCodecToString(
      const MediaCodec::VideoCodec& video_codec) {
    switch (video_codec) {
      case MediaCodec::VideoCodec::VP8:
        return "vp8";
      case MediaCodec::VideoCodec::H264:
        return "h264";
      case MediaCodec::VideoCodec::VP9:
        return "vp9";
      case MediaCodec::VideoCodec::H265:
        return "h265";
      default:
        RTC_NOTREACHED();
        return "";
    }
  }
}
}
