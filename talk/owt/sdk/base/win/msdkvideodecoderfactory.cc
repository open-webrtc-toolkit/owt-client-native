// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include <vector>
#include "absl/strings/match.h"
#include "media/base/codec.h"
#include "modules/video_coding/codecs/av1/libaom_av1_decoder.h"
#include "modules/video_coding/codecs/h264/include/h264.h"
#include "modules/video_coding/codecs/vp8/include/vp8.h"
#include "modules/video_coding/codecs/vp9/include/vp9.h"
#include "webrtc/rtc_base/checks.h"
#include "talk/owt/sdk/base/codecutils.h"
#include "talk/owt/sdk/base/win/msdkvideodecoderfactory.h"
#include "talk/owt/sdk/base/win/msdkvideodecoder.h"

namespace owt {
namespace base {

MSDKVideoDecoderFactory::MSDKVideoDecoderFactory() {
  supported_codec_types_.clear();
  // Always fallback to SW VP8 encoder as we don't want to
  // use the PAK + Shader one;
  supported_codec_types_.push_back(webrtc::kVideoCodecVP8);

  bool is_h264_hw_supported = true;
  if (is_h264_hw_supported) {
    supported_codec_types_.push_back(webrtc::kVideoCodecH264);
  }
  // Use SW AV1 until HW decoder in place. TODO: replace with
  // DXVA decoder or MSDK decoder on TGL and above.
  supported_codec_types_.push_back(webrtc::kVideoCodecAV1);
#ifndef DISABLE_H265
// TODO: Add logic to detect plugin by MSDK.
  bool is_h265_hw_supported = true;
  if (is_h265_hw_supported) {
    supported_codec_types_.push_back(webrtc::kVideoCodecH265);
  }
#endif

  MediaCapabilities* media_capability = MediaCapabilities::Get();
  std::vector<owt::base::VideoCodec> codecs_to_check;
  codecs_to_check.push_back(owt::base::VideoCodec::kH264);
  codecs_to_check.push_back(owt::base::VideoCodec::kVp9);
#ifndef DISABLE_H265
  codecs_to_check.push_back(owt::base::VideoCodec::kH265);
#endif
  codecs_to_check.push_back(owt::base::VideoCodec::kAv1);
  codecs_to_check.push_back(owt::base::VideoCodec::kVp8);
  std::vector<VideoDecoderCapability> capabilities =
      media_capability->SupportedCapabilitiesForVideoDecoder(codecs_to_check);

  for (auto& capability : capabilities) {
    if (capability.codec_type == owt::base::VideoCodec::kH264) {
      
    }
  }
}

MSDKVideoDecoderFactory::~MSDKVideoDecoderFactory() {
}

std::unique_ptr<webrtc::VideoDecoder> MSDKVideoDecoderFactory::CreateVideoDecoder(
    const webrtc::SdpVideoFormat& format) {

  if (absl::EqualsIgnoreCase(format.name, cricket::kVp8CodecName) ||
      absl::EqualsIgnoreCase(format.name, cricket::kH264CodecName)) {
    return MSDKVideoDecoder::Create(cricket::VideoCodec(format));
  } else if (absl::EqualsIgnoreCase(format.name, cricket::kVp9CodecName)) {
    return webrtc::VP9Decoder::Create();
  } else if (absl::EqualsIgnoreCase(format.name, cricket::kAv1CodecName)) {
    return webrtc::CreateLibaomAv1Decoder();
#ifndef DISABLE_H265
  } else if (absl::EqualsIgnoreCase(format.name, cricket::kH265CodecName)) {
    return MSDKVideoDecoder::Create(cricket::VideoCodec(format));
#endif
  }

  return nullptr;
}

 std::vector<webrtc::SdpVideoFormat> MSDKVideoDecoderFactory::GetSupportedFormats()
    const {
  std::vector<webrtc::SdpVideoFormat> supported_codecs;
  supported_codecs.push_back(webrtc::SdpVideoFormat(cricket::kVp8CodecName));
  for (const webrtc::SdpVideoFormat& format : webrtc::SupportedVP9Codecs())
    supported_codecs.push_back(format);
  for (const webrtc::SdpVideoFormat& format : owt::base::CodecUtils::SupportedH264Codecs())
    supported_codecs.push_back(format);
  if (webrtc::kIsLibaomAv1DecoderSupported) {
    supported_codecs.push_back(webrtc::SdpVideoFormat(cricket::kAv1CodecName));
  }
#ifndef DISABLE_H265
  for (const webrtc::SdpVideoFormat& format : CodecUtils::GetSupportedH265Codecs()) {
    supported_codecs.push_back(format);
  }
#endif
  return supported_codecs;
}
}  // namespace base
}  // namespace owt
