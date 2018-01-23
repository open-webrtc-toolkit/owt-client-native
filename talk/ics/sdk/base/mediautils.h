/*
 * Intel License
 */

#ifndef ICS_BASE_MEDIAUTILS_H_
#define ICS_BASE_MEDIAUTILS_H_

#include "talk/ics/sdk/include/cpp/ics/base/commontypes.h"

namespace ics {
namespace base {
class MediaUtils {
 public:
  static std::string GetResolutionName(const Resolution& resolution);
  static std::string AudioCodecToString(const AudioCodec& audio_codec);
  static std::string VideoCodecToString(const VideoCodec& video_codec);
  static AudioCodec GetAudioCodecFromString(const std::string& codec_name);
  static VideoCodec GetVideoCodecFromString(const std::string& codec_name);
};
}
}

#endif  // ICS_BASE_MEDIAUTILS_H_
