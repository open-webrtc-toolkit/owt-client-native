// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_ENCODEDVIDEOENCODERFACTORY_H_
#define OWT_BASE_ENCODEDVIDEOENCODERFACTORY_H_

#include <vector>
#include "webrtc/api/video_codecs/sdp_video_format.h"
#include "webrtc/api/video_codecs/video_encoder.h"
#include "webrtc/api/video_codecs/video_encoder_factory.h"

namespace owt {
namespace base {
class EncodedVideoEncoderFactory : public webrtc::VideoEncoderFactory {
 public:
  EncodedVideoEncoderFactory();
  virtual ~EncodedVideoEncoderFactory(){}
  using webrtc::VideoEncoderFactory::CreateVideoEncoder;

  std::unique_ptr<webrtc::VideoEncoder> CreateVideoEncoder(
      const webrtc::SdpVideoFormat& format) override;

  std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const override;

  webrtc::VideoEncoderFactory::CodecInfo QueryVideoEncoder(
      const webrtc::SdpVideoFormat& format) const override;
};

}  // namespace base
}  // namespace owt
#endif  // OWT_BASE_ENCODEDVIDEOENCODERFACTORY_H_
