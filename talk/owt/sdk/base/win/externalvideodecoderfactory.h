// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_WIN_MSDKVIDEODECODERFACTORY_H_
#define OWT_BASE_WIN_MSDKVIDEODECODERFACTORY_H_

#include <memory>
#include <vector>

#include <windows.h>
#include <d3d11.h>

#include "webrtc/api/video_codecs/sdp_video_format.h"
#include "webrtc/api/video/video_codec_type.h"
#include "webrtc/api/video_codecs/video_decoder.h"
#include "webrtc/api/video_codecs/video_decoder_factory.h"
#include "talk/owt/sdk/include/cpp/owt/base/commontypes.h"
#include "talk/owt/sdk/base/win/mediacapabilities.h"

namespace owt {
namespace base {
// Decoder factory for MSDK decoder and FFMPEG decoder. Only hardware decoders
// will be created.
class ExternalVideoDecoderFactory : public webrtc::VideoDecoderFactory {
 public:
  ExternalVideoDecoderFactory(ID3D11Device* d3d11_device_external);
  virtual ~ExternalVideoDecoderFactory();

  // VideoDecoderFactory implementation
  std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const override;

  std::unique_ptr<webrtc::VideoDecoder> CreateVideoDecoder(
      const webrtc::SdpVideoFormat& format) override;
 private:
  std::vector<webrtc::VideoCodecType> supported_codec_types_;
  ID3D11Device* external_device_ = nullptr;
  bool range_extension_enabled_ = false;
};
}  // namespace base
}  // namespace owt
#endif  // OWT_BASE_WIN_MSDKVIDEODECODERFACTORY_H_
