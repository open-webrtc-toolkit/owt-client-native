// Copyright (C) <2019> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_CODECUTILS_H_
#define OWT_BASE_CODECUTILS_H_

#include <vector>
#include "api/video_codecs/sdp_video_format.h"
#include "media/base/h264_profile_level_id.h"
namespace owt {
namespace base {
class CodecUtils {
 public:
  static std::vector<webrtc::SdpVideoFormat> SupportedH264Codecs();
#ifndef DISABLE_H265
  static std::vector<webrtc::SdpVideoFormat> GetSupportedH265Codecs();
#endif
};
}
}
#endif  // OWT_BASE_CODECUTILS_H_
