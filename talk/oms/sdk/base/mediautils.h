// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OMS_BASE_MEDIAUTILS_H_
#define OMS_BASE_MEDIAUTILS_H_
#include "talk/oms/sdk/include/cpp/oms/base/commontypes.h"
namespace oms {
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
#endif  // OMS_BASE_MEDIAUTILS_H_
