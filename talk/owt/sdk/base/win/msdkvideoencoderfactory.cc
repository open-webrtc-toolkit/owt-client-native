// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/base/win/msdkvideoencoderfactory.h"
#include <string>
#include "absl/strings/match.h"
#include "modules/video_coding/codecs/av1/libaom_av1_encoder.h"
#include "modules/video_coding/codecs/h264/include/h264.h"
#include "modules/video_coding/codecs/vp8/include/vp8.h"
#include "modules/video_coding/codecs/vp9/include/vp9.h"
#include "talk/owt/sdk/base/codecutils.h"
#include "talk/owt/sdk/base/win/msdkvideoencoder.h"
#include "webrtc/common_video/h264/profile_level_id.h"
#include "webrtc/media/base/vp9_profile.h"

namespace owt {
namespace base {

std::unique_ptr<webrtc::VideoEncoder> MSDKVideoEncoderFactory::CreateVideoEncoder(
    const webrtc::SdpVideoFormat& format) {
  // VP8 encoding will always use SW impl.
  if (absl::EqualsIgnoreCase(format.name, cricket::kVp8CodecName))
    return webrtc::VP8Encoder::Create();
  // VP9 encoding will only be enabled on ICL+;
  else if (absl::EqualsIgnoreCase(format.name, cricket::kVp9CodecName))
    return webrtc::VP9Encoder::Create(cricket::VideoCodec(format));
  else if (absl::EqualsIgnoreCase(format.name, cricket::kH264CodecName))
    return MSDKVideoEncoder::Create(cricket::VideoCodec(format));
  // TODO: Replace with AV1 HW encoder post TGL.
  else if (absl::EqualsIgnoreCase(format.name, cricket::kAv1CodecName))
    return webrtc::CreateLibaomAv1Encoder();
#ifndef DISABLE_H265
  else if (absl::EqualsIgnoreCase(format.name, cricket::kH265CodecName))
    return MSDKVideoEncoder::Create(cricket::VideoCodec(format));
#endif
  return nullptr;
}

std::vector<webrtc::SdpVideoFormat>
MSDKVideoEncoderFactory::GetSupportedFormats() const {
  std::vector<webrtc::SdpVideoFormat> supported_codecs;
  supported_codecs.push_back(webrtc::SdpVideoFormat(cricket::kVp8CodecName));
  for (const webrtc::SdpVideoFormat& format : webrtc::SupportedVP9Codecs())
    supported_codecs.push_back(format);
  // TODO: We should combine the codec profiles that hardware H.264 encoder
  // supports with those provided by built-in H.264 encoder
  for (const webrtc::SdpVideoFormat& format : owt::base::CodecUtils::SupportedH264Codecs())
    supported_codecs.push_back(format);
  if (webrtc::kIsLibaomAv1EncoderSupported) {
    supported_codecs.push_back(webrtc::SdpVideoFormat(cricket::kAv1CodecName));
  }
#ifndef DISABLE_H265
  for (const webrtc::SdpVideoFormat& format : CodecUtils::GetSupportedH265Codecs()) {
    supported_codecs.push_back(format);
  }
#endif
  return supported_codecs;
}

webrtc::VideoEncoderFactory::CodecInfo
MSDKVideoEncoderFactory::QueryVideoEncoder(
    const webrtc::SdpVideoFormat& format) const {
  webrtc::VideoEncoderFactory::CodecInfo info;
  info.is_hardware_accelerated = true;
  info.has_internal_source = false;
  return info;
}

}  // namespace base
}  // namespace owt
