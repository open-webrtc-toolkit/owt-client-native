/*
 * Intel License
 */

#ifndef WOOGEEN_BASE_SDPUTILS_H_
#define WOOGEEN_BASE_SDPUTILS_H_

#include <string>
#include "talk/woogeen/sdk/include/cpp/woogeen/base/mediaformat.h"

namespace woogeen {
namespace base {
/// This class provides utilities for SDP parsing and modification
class SdpUtils {
 public:
  /// Set max audio bandwidth, unit is Kbps.
  static std::string SetMaximumAudioBandwidth(std::string sdp, int bitrate);
  /// Set max video bandwidth, unit is Kbps.
  static std::string SetMaximumVideoBandwidth(std::string sdp, int bitrate);
  static std::string SetPreferVideoCodec(std::string sdp,
                                         MediaCodec::VideoCodec codec);
};
}
}
#endif  // WOOGEEN_BASE_SDPUTILS_H_
