// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_WIN_MSDKVIDEOENCODER_FACTORY_H_
#define OWT_BASE_WIN_MSDKVIDEOENCODER_FACTORY_H_

#include <vector>
#include "webrtc/api/video_codecs/video_encoder.h"
#include "webrtc/api/video_codecs/sdp_video_format.h"
#include "webrtc/api/video_codecs/video_encoder_factory.h"
#include "talk/owt/sdk/base/win/mediacapabilities.h"

namespace owt {
namespace base {
class MSDKVideoEncoderFactory : public webrtc::VideoEncoderFactory {
 public:
  std::unique_ptr<webrtc::VideoEncoder> CreateVideoEncoder(
      const webrtc::SdpVideoFormat& format) override;

  std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const override;

  webrtc::VideoEncoderFactory::CodecInfo QueryVideoEncoder(
      const webrtc::SdpVideoFormat& format) const override;
};
}  // namespace base
}  // namespace owt
#endif  // OWT_BASE_WIN_MSDKVIDEOENCODER_FACTORY_H_