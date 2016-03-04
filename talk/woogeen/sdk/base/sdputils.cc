/*
 * Intel License
 */

#include <regex>
#include <sstream>
#include "talk/woogeen/sdk/base/sdputils.h"

namespace woogeen {
namespace base {

std::string SdpUtils::SetMaximumVideoBandwidth(std::string sdp, int bitrate) {
  std::regex reg("a=mid:video\r\n");
  std::stringstream sdp_line_width_bitrate;
  sdp_line_width_bitrate << "a=mid:video\r\nb=AS: " << bitrate << "\r\n";
  return std::regex_replace(sdp, reg, sdp_line_width_bitrate.str());
}

std::string SdpUtils::SetMaximumAudioBandwidth(std::string sdp, int bitrate) {
  std::regex reg("a=mid:audio\r\n");
  std::stringstream sdp_line_width_bitrate;
  sdp_line_width_bitrate << "a=mid:audio\r\nb=AS: " << bitrate << "\r\n";
  return std::regex_replace(sdp, reg, sdp_line_width_bitrate.str());
}

std::string SdpUtils::SetPreferVideoCodec(std::string sdp,
                                          MediaCodec::VideoCodec codec) {
  // TODO: Change codec order here. Originally, we change codec order in
  // PeerConnectionChannel.
  return sdp;
}
}
}
