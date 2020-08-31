// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_MEDIAUTILS_H_
#define OWT_BASE_MEDIAUTILS_H_

#include "absl/types/optional.h"
#include "api/video_codecs/sdp_video_format.h"
#include <string>
#include "talk/owt/sdk/base/win/mediacapabilities.h"
#include "talk/owt/sdk/include/cpp/owt/base/commontypes.h"

namespace owt {
namespace base {


class MediaUtils {
 public:
  static std::string GetResolutionName(const Resolution& resolution);
  static std::string AudioCodecToString(const AudioCodec& audio_codec);
  static std::string VideoCodecToString(const VideoCodec& video_codec);
  static AudioCodec GetAudioCodecFromString(const std::string& codec_name);
  static VideoCodec GetVideoCodecFromString(const std::string& codec_name);
  static absl::optional<unsigned int> GetH264TemporalLayers();
  static bool GetH264TemporalInfo(uint8_t* buffer,
                                  size_t buffer_length,
                                  int& temporal_id,
                                  int& priority_id,
                                  bool& is_idr);
  static absl::optional<AV1Profile> ParseSdpForAV1Profile(
      const webrtc::SdpVideoFormat::Parameters& params);
};
}
}
#endif  // OWT_BASE_MEDIAUTILS_H_
