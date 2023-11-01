// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/base/win/externalvideodecoderfactory.h"
#include <vector>
#include "absl/strings/match.h"
#include "media/base/codec.h"
#include "modules/video_coding/codecs/av1/dav1d_decoder.h"
#include "modules/video_coding/codecs/h264/include/h264.h"
#include "modules/video_coding/codecs/vp8/include/vp8.h"
#include "modules/video_coding/codecs/vp9/include/vp9.h"
#include "talk/owt/sdk/base/codecutils.h"
#ifdef OWT_USE_FFMPEG
#include "talk/owt/sdk/base/win/d3d11_video_decoder.h"
#include "talk/owt/sdk/base/win/ffmpeg_decoder_impl.h"
#endif
#ifdef OWT_USE_MSDK
#include "talk/owt/sdk/base/win/msdkvideodecoder.h"
#endif
#include "system_wrappers/include/field_trial.h"
#include "webrtc/rtc_base/checks.h"
#include "webrtc/rtc_base/logging.h"

namespace owt {
namespace base {

ExternalVideoDecoderFactory::ExternalVideoDecoderFactory(ID3D11Device* d3d11_device_external) {
  range_extension_enabled_ =
      webrtc::field_trial::IsEnabled("OWT-RangeExtension");
  supported_codec_types_.clear();

  bool is_vp8_hw_supported = false, is_vp9_hw_supported = false;
  bool is_h264_hw_supported = false, is_av1_hw_supported = false;
#ifdef WEBRTC_USE_H265
  // TODO: Add logic to detect plugin by MSDK.
  bool is_h265_hw_supported = true;
#endif

  MediaCapabilities* media_capability = MediaCapabilities::Get();
  std::vector<owt::base::VideoCodec> codecs_to_check;
  codecs_to_check.push_back(owt::base::VideoCodec::kH264);
  codecs_to_check.push_back(owt::base::VideoCodec::kVp9);
#ifdef WEBRTC_USE_H265
  codecs_to_check.push_back(owt::base::VideoCodec::kH265);
#endif
  codecs_to_check.push_back(owt::base::VideoCodec::kAv1);
  codecs_to_check.push_back(owt::base::VideoCodec::kVp8);
  std::vector<VideoDecoderCapability> capabilities =
      media_capability->SupportedCapabilitiesForVideoDecoder(codecs_to_check);

  for (auto& capability : capabilities) {
    if (capability.codec_type == owt::base::VideoCodec::kH264 && !is_h264_hw_supported) {
      is_h264_hw_supported = true;
      supported_codec_types_.push_back(webrtc::kVideoCodecH264);
    } else if (capability.codec_type == owt::base::VideoCodec::kVp9 && !is_vp9_hw_supported) {
      is_vp9_hw_supported = true;
      supported_codec_types_.push_back(webrtc::kVideoCodecVP9);
    } else if (capability.codec_type == owt::base::VideoCodec::kVp8 && !is_vp8_hw_supported) {
      is_vp8_hw_supported = true;
      supported_codec_types_.push_back(webrtc::kVideoCodecVP8);
    } else if (capability.codec_type == owt::base::VideoCodec::kAv1 && !is_av1_hw_supported) {
      is_av1_hw_supported = true;
      supported_codec_types_.push_back(webrtc::kVideoCodecAV1);
    }
#ifdef WEBRTC_USE_H265
    else if (capability.codec_type == owt::base::VideoCodec::kH265 && !is_h265_hw_supported) {
      is_h265_hw_supported = true;
      supported_codec_types_.push_back(webrtc::kVideoCodecH265);
    }
#endif
    external_device_ = d3d11_device_external;
  }
}

ExternalVideoDecoderFactory::~ExternalVideoDecoderFactory() {}

std::unique_ptr<webrtc::VideoDecoder>
ExternalVideoDecoderFactory::CreateVideoDecoder(
    const webrtc::SdpVideoFormat& format) {
  bool vp9_hw = false, vp8_hw = false, av1_hw = false, h264_hw = false;
#ifdef WEBRTC_USE_H265
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
#ifdef WEBRTC_USE_H265
    else if (codec == webrtc::kVideoCodecH265) {
      h265_hw = true;
    }
#endif
  }
  if (absl::EqualsIgnoreCase(format.name, cricket::kVp9CodecName) && !vp9_hw) {
    return webrtc::VP9Decoder::Create();
  } else if (absl::EqualsIgnoreCase(format.name, cricket::kVp8CodecName) &&
             !vp8_hw) {
    RTC_LOG(LS_INFO)
        << "Not supporting HW VP8 decoder. Requesting SW decoding.";
    return webrtc::VP8Decoder::Create();
  }  else if (absl::EqualsIgnoreCase(format.name, cricket::kH264CodecName) &&
           !h264_hw) {
    return webrtc::H264Decoder::Create();
  } else if (absl::EqualsIgnoreCase(format.name, cricket::kAv1CodecName) &&
           !av1_hw) {
    return webrtc::CreateDav1dDecoder();
  }
  if (vp8_hw || vp9_hw || h264_hw || h265_hw || av1_hw) {
#if defined(OWT_USE_FFMPEG)
    if (range_extension_enabled_) {
      return std::make_unique<FFMpegDecoderImpl>();
    } else {
      return owt::base::D3D11VideoDecoder::Create(cricket::VideoCodec(format));
    }
#endif
#if defined(OWT_USE_MSDK)
    return owt::base::MSDKVideoDecoder::Create(cricket::VideoCodec(format));
#endif
  }

  RTC_CHECK_NOTREACHED();
}

 std::vector<webrtc::SdpVideoFormat> ExternalVideoDecoderFactory::GetSupportedFormats()
    const {
  std::vector<webrtc::SdpVideoFormat> supported_codecs;
  supported_codecs.push_back(webrtc::SdpVideoFormat(cricket::kVp8CodecName));
  supported_codecs.push_back(webrtc::SdpVideoFormat(cricket::kAv1CodecName));
  for (const webrtc::SdpVideoFormat& format : webrtc::SupportedVP9Codecs())
    supported_codecs.push_back(format);
  for (const webrtc::SdpVideoFormat& format : owt::base::CodecUtils::SupportedH264Codecs())
    supported_codecs.push_back(format);
#ifdef WEBRTC_USE_H265
  for (const webrtc::SdpVideoFormat& format : CodecUtils::GetSupportedH265Codecs()) {
    supported_codecs.push_back(format);
  }
#endif
  return supported_codecs;
}

}  // namespace base
}  // namespace owt
