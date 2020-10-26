// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
//
#include "absl/strings/match.h"
#include "media/base/codec.h"
#include "modules/video_coding/codecs/h264/include/h264.h"
#include "modules/video_coding/codecs/vp8/include/vp8.h"
#include "modules/video_coding/codecs/vp9/include/vp9.h"
#include "webrtc/rtc_base/checks.h"
#include "talk/owt/sdk/base/codecutils.h"
#include "talk/owt/sdk/base/linux/msdkvideodecoderfactory.h"
#include "talk/owt/sdk/base/linux/msdkvideodecoder.h"

namespace owt {
namespace base {

MSDKVideoDecoderFactory::MSDKVideoDecoderFactory() {
  // TODO: Add the other codecs support
  supported_codecs_.push_back(webrtc::kVideoCodecH264);
  supported_codecs_.push_back(webrtc::kVideoCodecH265);
}



MSDKVideoDecoderFactory::~MSDKVideoDecoderFactory() {}


std::unique_ptr<webrtc::VideoDecoder> MSDKVideoDecoderFactory::CreateVideoDecoder(
    const webrtc::SdpVideoFormat& format) {
 
  if (absl::EqualsIgnoreCase(format.name, cricket::kVp8CodecName) ||
      absl::EqualsIgnoreCase(format.name, cricket::kH264CodecName)) {
      return (std::unique_ptr<webrtc::VideoDecoder>)new MsdkVideoDecoder();
  } else if (absl::EqualsIgnoreCase(format.name, cricket::kVp9CodecName)) {
    return webrtc::VP9Decoder::Create();
#ifndef DISABLE_H265
  } else if (absl::EqualsIgnoreCase(format.name, cricket::kH265CodecName)) {
    //return MSDKVideoDecoder::Create(cricket::VideoCodec(format));
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
#ifndef DISABLE_H265
  for (const webrtc::SdpVideoFormat& format : CodecUtils::GetSupportedH265Codecs()) {
    supported_codecs.push_back(format);
  }
#endif
  return supported_codecs;
}

}  // namespace base
}  // namespace owt
