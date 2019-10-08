// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_CUSTOMIZEDVIDEODECODERFACTORY_H_
#define OWT_BASE_CUSTOMIZEDVIDEODECODERFACTORY_H_

#include <vector>

#include "talk/owt/sdk/base/customizedvideodecoderproxy.h"
#include "talk/owt/sdk/include/cpp/owt/base/videodecoderinterface.h"
#include "webrtc/api/video_codecs/sdp_video_format.h"
#include "webrtc/api/video_codecs/video_decoder.h"
#include "webrtc/api/video_codecs/video_decoder_factory.h"

namespace owt {
namespace base {
// Declaration of customized based decoder factory.
class CustomizedVideoDecoderFactory : public webrtc::VideoDecoderFactory {
 public:
  CustomizedVideoDecoderFactory(
      std::unique_ptr<owt::base::VideoDecoderInterface> external_decoder);
  virtual ~CustomizedVideoDecoderFactory();
  // WebRtcVideoDecoderFactory implementation.
  std::unique_ptr<webrtc::VideoDecoder> CreateVideoDecoder(
      const webrtc::SdpVideoFormat& format) override;
  std::vector<SdpVideoFormat> GetSupportedFormats() const override;
 private:
  std::unique_ptr<owt::base::VideoDecoderInterface> external_decoder_;
};

}  // namespace base
}  // namespace owt
#endif  // OWT_BASE_CUSTOMIZEDVIDEODECODERFACTORY_H_