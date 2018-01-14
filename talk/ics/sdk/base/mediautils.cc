/*
 * Intel License
 */

#include <map>
#include <string>
#include "talk/ics/sdk/base/mediautils.h"
#include "webrtc/rtc_base/checks.h"

namespace ics {
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
      const AudioCodec& audio_codec) {
    switch (audio_codec) {
      case AudioCodec::kOPUS:
        return "opus";
      case AudioCodec::kISAC:
        return "isac";
      case AudioCodec::kG722:
        return "g722";
      case AudioCodec::kPCMU:
        return "pcmu";
      case AudioCodec::kPCMA:
        return "pcma";
      default:
        RTC_NOTREACHED();
        return "";
    }
  }

  std::string MediaUtils::VideoCodecToString(
      const VideoCodec& video_codec) {
    switch (video_codec) {
      case VideoCodec::kVP8:
        return "vp8";
      case VideoCodec::kH264:
        return "h264";
      case VideoCodec::kVP9:
        return "vp9";
      case VideoCodec::kH265:
        return "h265";
      default:
        RTC_NOTREACHED();
        return "";
    }
  }
}
}
