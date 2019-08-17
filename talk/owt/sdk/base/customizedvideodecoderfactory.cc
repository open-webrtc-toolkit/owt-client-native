// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "absl/strings/match.h"
#include "media/base/codec.h"
#include "modules/video_coding/codecs/h264/include/h264.h"
#include "modules/video_coding/codecs/vp8/include/vp8.h"
#include "modules/video_coding/codecs/vp9/include/vp9.h"
#include "owt/base/globalconfiguration.h"
#include "talk/owt/sdk/base/customizedvideodecoderfactory.h"
#include "talk/owt/sdk/base/customizedvideodecoderproxy.h"

namespace owt {
namespace base {
CustomizedVideoDecoderFactory::CustomizedVideoDecoderFactory(
    std::unique_ptr<owt::base::VideoDecoderInterface> external_decoder)
    : external_decoder_(std::move(external_decoder)) {
}

CustomizedVideoDecoderFactory::~CustomizedVideoDecoderFactory() {}

std::unique_ptr<webrtc::VideoDecoder>
CustomizedVideoDecoderFactory::CreateVideoDecoder(
    const webrtc::SdpVideoFormat& format) {
  return CustomizedVideoDecoderProxy::Create(external_decoder_->Copy());
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

 std::vector<SdpVideoFormat>CustomizedVideoDecoderFactory::GetSupportedFormats() const {
  std::vector<SdpVideoFormat> supported_codecs;
  supported_codecs.push_back(SdpVideoFormat(cricket::kVp8CodecName));
  for (const webrtc::SdpVideoFormat& format : webrtc::SupportedVP9Codecs())
    supported_codecs.push_back(format);
  for (const webrtc::SdpVideoFormat& format : webrtc::SupportedH264Codecs())
    supported_codecs.push_back(format);
#ifndef DISABLE_H265
  for (const webrtc::SdpVideoFormat& format : GetSupportedH265Codecs()) {
    supported_codecs.push_back(format);
  }
#endif
  return supported_codecs;
 }

} //namespace base
} //namespace owt