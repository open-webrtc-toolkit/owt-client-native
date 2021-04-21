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

MSDKVideoEncoderFactory::MSDKVideoEncoderFactory() {
  supported_codec_types_.clear();
  MediaCapabilities* media_capability = MediaCapabilities::Get();
  std::vector<owt::base::VideoCodec> codecs_to_check;
  codecs_to_check.push_back(owt::base::VideoCodec::kH264);
  codecs_to_check.push_back(owt::base::VideoCodec::kVp9);
#ifndef DISABLE_H265
  codecs_to_check.push_back(owt::base::VideoCodec::kH265);
#endif
  codecs_to_check.push_back(owt::base::VideoCodec::kAv1);
  codecs_to_check.push_back(owt::base::VideoCodec::kVp8);
  std::vector<VideoEncoderCapability> capabilities =
      media_capability->SupportedCapabilitiesForVideoEncoder(codecs_to_check);
  // TODO(jianlin): use the check result from MSDK.
  supported_codec_types_.push_back(webrtc::kVideoCodecH264);
  supported_codec_types_.push_back(webrtc::kVideoCodecVP9);
  supported_codec_types_.push_back(webrtc::kVideoCodecVP8);
#ifndef DISABLE_H265
  supported_codec_types_.push_back(webrtc::kVideoCodecH265);
#endif
  supported_codec_types_.push_back(webrtc::kVideoCodecAV1);
}

std::unique_ptr<webrtc::VideoEncoder> MSDKVideoEncoderFactory::CreateVideoEncoder(
    const webrtc::SdpVideoFormat& format) {
  bool vp9_hw = false, vp8_hw = false, av1_hw = false, h264_hw = false;
#ifndef DISABLE_H265
  bool h265_hw = false;
#endif
  for (auto& codec : supported_codec_types_) {
    if (codec == webrtc::kVideoCodecAV1)
      av1_hw = true;
    else if (codec == webrtc::kVideoCodecH264)
      h264_hw = true;
    else if (codec == webrtc::kVideoCodecVP8)
      vp8_hw = true;
    else if (codec == webrtc::kVideoCodecVP9)
      vp9_hw = true;
#ifndef DISABLE_H265
    else if (codec == webrtc::kVideoCodecH265)
      h265_hw = true;
#endif
  }
  // VP8 encoding will always use SW impl.
  if (absl::EqualsIgnoreCase(format.name, cricket::kVp8CodecName) && !vp8_hw)
    return webrtc::VP8Encoder::Create();
  // VP9 encoding will only be enabled on ICL+;
  else if (absl::EqualsIgnoreCase(format.name, cricket::kVp9CodecName))
    return webrtc::VP9Encoder::Create(cricket::VideoCodec(format));
  // TODO: Replace with AV1 HW encoder post ADL.
  else if (absl::EqualsIgnoreCase(format.name, cricket::kAv1CodecName))
    return webrtc::CreateLibaomAv1Encoder();
#ifndef DISABLE_H265
  else if (absl::EqualsIgnoreCase(format.name, cricket::kH265CodecName) &&
           !h265_hw) {
  }
#endif
  return MSDKVideoEncoder::Create(cricket::VideoCodec(format));
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
  info.has_internal_source = false;
  return info;
}

}  // namespace base
}  // namespace owt
