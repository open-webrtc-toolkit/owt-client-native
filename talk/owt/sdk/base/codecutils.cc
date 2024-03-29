// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/base/codecutils.h"
#include "absl/memory/memory.h"
#include "absl/strings/match.h"
#include "absl/types/optional.h"
#include "api/video_codecs/h264_profile_level_id.h"
#include "media/base/media_constants.h"

namespace owt {
namespace base {

webrtc::SdpVideoFormat CreateH264Format(webrtc::H264Profile profile,
                                        webrtc::H264Level level,
                                        const std::string& packetization_mode) {
  const absl::optional<std::string> profile_string =
      webrtc::H264ProfileLevelIdToString(
          webrtc::H264ProfileLevelId(profile, level));
  return webrtc::SdpVideoFormat(
      cricket::kH264CodecName,
      {{cricket::kH264FmtpProfileLevelId, *profile_string},
       {cricket::kH264FmtpLevelAsymmetryAllowed, "1"},
       {cricket::kH264FmtpPacketizationMode, packetization_mode}});
}

std::vector<webrtc::SdpVideoFormat> CodecUtils::SupportedH264Codecs() {
  return {CreateH264Format(webrtc::H264Profile::kProfileBaseline,
                           webrtc::H264Level::kLevel3_1, "1"),
          CreateH264Format(webrtc::H264Profile::kProfileBaseline,
                           webrtc::H264Level::kLevel3_1, "0"),
          CreateH264Format(webrtc::H264Profile::kProfileConstrainedBaseline,
                           webrtc::H264Level::kLevel3_1, "1"),
          CreateH264Format(webrtc::H264Profile::kProfileConstrainedBaseline,
                           webrtc::H264Level::kLevel3_1, "0")};
}

#ifdef WEBRTC_USE_H265
std::vector<webrtc::SdpVideoFormat> CodecUtils::GetSupportedH265Codecs() {
  return {webrtc::SdpVideoFormat(cricket::kH265CodecName,
                                 {{cricket::kH265FmtpProfileSpace, "0"},
                                  {cricket::kH265FmtpProfileId, "1"},
                                  {cricket::kH265FmtpTierFlag, "0"},
                                  {cricket::kH265FmtpLevelId, "120"}}),
          webrtc::SdpVideoFormat(cricket::kH265CodecName,
                                 {{cricket::kH265FmtpProfileSpace, "0"},
                                  {cricket::kH265FmtpProfileId, "2"},
                                  {cricket::kH265FmtpTierFlag, "0"},
                                  {cricket::kH265FmtpLevelId, "120"}})};
}
#endif
webrtc::VideoCodecType CodecUtils::ConvertSdpFormatToCodecType(
    webrtc::SdpVideoFormat format) {
  if (absl::EqualsIgnoreCase(format.name, cricket::kVp8CodecName)) {
    return webrtc::kVideoCodecVP8;
  } else if (absl::EqualsIgnoreCase(format.name, cricket::kVp9CodecName)) {
    return webrtc::kVideoCodecVP9;
  } else if (absl::EqualsIgnoreCase(format.name, cricket::kH264CodecName)) {
    return webrtc::kVideoCodecH264;
#ifdef WEBRTC_USE_H265
  } else if (absl::EqualsIgnoreCase(format.name, cricket::kH265CodecName)) {
    return webrtc::kVideoCodecH265;
#endif
  } else if (absl::EqualsIgnoreCase(format.name, cricket::kAv1CodecName)) {
    return webrtc::kVideoCodecAV1;
  } else {
    return webrtc::kVideoCodecGeneric;
  }
}
}  // namespace base
}  // namespace owt
