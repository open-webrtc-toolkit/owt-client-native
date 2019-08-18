// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "absl/strings/match.h"
#include "modules/video_coding/codecs/h264/include/h264.h"
#include "modules/video_coding/codecs/vp8/include/vp8.h"
#include "modules/video_coding/codecs/vp9/include/vp9.h"
#include "webrtc/common_types.h"
#include "webrtc/common_video/h264/profile_level_id.h"
#include "webrtc/rtc_base/string_utils.h"
#include "talk/owt/sdk/base/customizedvideoencoderproxy.h"
#include "talk/owt/sdk/base/encodedvideoencoderfactory.h"

namespace owt {
namespace base {

EncodedVideoEncoderFactory::EncodedVideoEncoderFactory() {}

std::unique_ptr<webrtc::VideoEncoder>
EncodedVideoEncoderFactory::CreateVideoEncoder(
    const webrtc::SdpVideoFormat& format) {
  if (absl::EqualsIgnoreCase(format.name, cricket::kVp8CodecName))
    return webrtc::VP8Encoder::Create();
  if (absl::EqualsIgnoreCase(format.name, cricket::kVp9CodecName))
    return webrtc::VP9Encoder::Create(cricket::VideoCodec(format));
  if (absl::EqualsIgnoreCase(format.name, cricket::kH264CodecName))
    return CustomizedVideoEncoderProxy::Create();
    // return webrtc::H264Encoder::Create(cricket::VideoCodec(format));
#ifndef DISABLE_H265
  if (absl::EqualsIgnoreCase(format.name, cricket::kH265Codecname))
    return MSDKVideoEncoder::Create(cricket::VideoCodec(format));
#endif
}

std::vector<webrtc::SdpVideoFormat>
EncodedVideoEncoderFactory::GetSupportedFormats() const {
  std::vector<webrtc::SdpVideoFormat> supported_codecs;
  supported_codecs.push_back(webrtc::SdpVideoFormat(cricket::kVp8CodecName));
  for (const webrtc::SdpVideoFormat& format : webrtc::SupportedVP9Codecs())
    supported_codecs.push_back(format);
  // TODO: We should combine the codec profiles that hardware H.264 encoder
  // supports with those provided by built-in H.264 encoder
  for (const webrtc::SdpVideoFormat& format : webrtc::SupportedH264Codecs())
    supported_codecs.push_back(format);
#ifndef DISABLE_H265
  for (const webrtc::SdpVideoFormat& format : GetSupportedH265Codecs()) {
    supported_codecs.push_back(format);
  }
#endif
}

webrtc::VideoEncoderFactory::CodecInfo
EncodedVideoEncoderFactory::QueryVideoEncoder(
    const webrtc::SdpVideoFormat& format) const {
  // TODO(johny): Basically we need to return different CodecInfo for different
  // codec/profile combinations.
  webrtc::VideoEncoderFactory::CodecInfo info;
  info.is_hardware_accelerated = false;
  info.has_internal_source = false;
  return info;
}

#ifndef DISABLE_H265
std::vector<webrtc::SdpVideoFormat> GetSupportedH265Codecs() {
  return {webrtc::SdpVideoFormat(cricket::kH265CodecName,
                                 {{cricket::kH265FmtpProfileSpace, "0"},
                                  {cricket::kH265FmtpProfileId, "1"},
                                  {cricket::kH265FmtpTierFlag, "0"},
                                  {cricket::kH265FmtpLevelId, "120"}})};
}
#endif

} // namespace base
} // namespace owt
