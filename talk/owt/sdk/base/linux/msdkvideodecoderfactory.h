// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_LINUX_MSDKVIDEODECODERFACTORY_H_
#define OWT_BASE_LINUX_MSDKVIDEODECODERFACTORY_H_

#include <vector>

#include "webrtc/api/video_codecs/sdp_video_format.h"
#include "webrtc/api/video_codecs/video_decoder.h"
#include "webrtc/api/video_codecs/video_decoder_factory.h"
namespace owt {
namespace base {

// Declaration of MSDK based decoder factory.
class MSDKVideoDecoderFactory : public webrtc::VideoDecoderFactory {
 public:
  MSDKVideoDecoderFactory();
  virtual ~MSDKVideoDecoderFactory();

  // WebRtcVideoDecoderFactory implementation.
  // VideoDecoderFactory implementation
  std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const override;

  std::unique_ptr<webrtc::VideoDecoder> CreateVideoDecoder(
      const webrtc::SdpVideoFormat& format) override;


 private:
  std::vector<webrtc::VideoCodecType> supported_codecs_;
};

}  // namespace base
}  // namespace owt

#endif  // OWT_BASE_LINUX_MSDKVIDEODECODERFACTORY_H_

