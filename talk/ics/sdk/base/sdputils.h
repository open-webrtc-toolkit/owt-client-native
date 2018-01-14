/*
 * Intel License
 */

#ifndef ICS_BASE_SDPUTILS_H_
#define ICS_BASE_SDPUTILS_H_

#include <string>
#include <vector>
#include "talk/ics/sdk/include/cpp/ics/base/common_types.h"

namespace ics {
namespace base {
/// This class provides utilities for SDP parsing and modification
class SdpUtils {
 public:
  static std::string SetPreferAudioCodec(const std::string& sdp,
                                         AudioCodec codec);
  static std::string SetPreferVideoCodec(const std::string& sdp,
                                         VideoCodec codec);

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
#endif  // ICS_BASE_SDPUTILS_H_
