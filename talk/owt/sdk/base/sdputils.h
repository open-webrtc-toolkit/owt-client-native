// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_SDPUTILS_H_
#define OWT_BASE_SDPUTILS_H_
#include <string>
#include <vector>
#include "talk/owt/sdk/include/cpp/owt/base/commontypes.h"
namespace owt {
namespace base {
/// This class provides utilities for SDP parsing and modification
class SdpUtils {
 public:
  static std::string SetPreferAudioCodecs(const std::string& sdp,
                                         std::vector<AudioCodec>& codec);
  static std::string SetPreferVideoCodecs(const std::string& sdp,
                                         std::vector<VideoCodec>& codec);
  static std::string SetStartVideoBandwidth(const std::string& sdp,
                                            int bandwidth);
 private:
  /**
   @brief Replace SDP for preferred codec.
   @param sdp Original SDP.
   @param codec_names Codec names in SDP.
   @param is_audio True if prefer audio codec, false if prefer video codec.
   */
  static std::string SetPreferCodecs(const std::string& sdp,
                                     std::vector<std::string>& codec_name,
                                     bool is_audio);
  static std::vector<std::string> GetCodecValues(const std::string& sdp,
                                                 std::string& codec_name,
                                                 bool is_audio);
};
}
}
#endif  // OWT_BASE_SDPUTILS_H_
