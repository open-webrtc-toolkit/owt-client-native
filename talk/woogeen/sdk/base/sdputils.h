/*
 * Intel License
 */

#ifndef WOOGEEN_BASE_SDPUTILS_H_
#define WOOGEEN_BASE_SDPUTILS_H_

#include <string>
#include <vector>
#include "talk/woogeen/sdk/include/cpp/woogeen/base/mediaformat.h"

namespace woogeen {
namespace base {
/// This class provides utilities for SDP parsing and modification
class SdpUtils {
 public:
  /// Set max audio bandwidth, unit is Kbps.
  static std::string SetMaximumAudioBandwidth(const std::string& sdp,
                                              int bitrate);
  /// Set max video bandwidth, unit is Kbps.
  static std::string SetMaximumVideoBandwidth(const std::string& sdp,
                                              int bitrate);
  static std::string SetPreferAudioCodec(const std::string& sdp,
                                         MediaCodec::AudioCodec codec);
  static std::string SetPreferVideoCodec(const std::string& sdp,
                                         MediaCodec::VideoCodec codec);

 private:
  /**
   @brief Replace SDP for preferred codec.
   @param sdp Original SDP.
   @param codec_name Codec name in SDP.
   @param is_audio True if prefer audio codec, false if prefer video codec.
   */
  static std::string SetPreferCodec(const std::string& sdp,
                                    const std::string& codec_name,
                                    bool is_audio);
};
}
}
#endif  // WOOGEEN_BASE_SDPUTILS_H_
