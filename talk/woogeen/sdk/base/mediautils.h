/*
 * Intel License
 */

#ifndef WOOGEEN_BASE_MEDIAUTILS_H_
#define WOOGEEN_BASE_MEDIAUTILS_H_

#include "talk/woogeen/sdk/include/cpp/woogeen/base/mediaformat.h"

namespace woogeen {
namespace base {
class MediaUtils {
 public:
  static std::string GetResolutionName(const Resolution& resolution);
  static std::string AudioCodecToString(const MediaCodec::AudioCodec& audio_codec);
  static std::string VideoCodecToString(const MediaCodec::VideoCodec& video_codec);
};
}
}

#endif  // WOOGEEN_BASE_MEDIAUTILS_H_
