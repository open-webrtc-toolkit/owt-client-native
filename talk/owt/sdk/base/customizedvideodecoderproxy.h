// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_CUSTOMIZEDVIDEODECODERPROXY_H_
#define OWT_BASE_CUSTOMIZEDVIDEODECODERPROXY_H_

#include <vector>
#include "media/base/codec.h"
#include "webrtc/modules/video_coding/include/video_codec_interface.h"
#include "webrtc/system_wrappers/include/clock.h"
#include "talk/owt/sdk/include/cpp/owt/base/videodecoderinterface.h"

namespace owt {
namespace base {
using namespace webrtc;
class CustomizedVideoDecoderProxy : public VideoDecoder {
 public:
  static std::unique_ptr<CustomizedVideoDecoderProxy> Create(
      VideoDecoderInterface* external_video_decoder);
  explicit CustomizedVideoDecoderProxy(
      VideoDecoderInterface* external_video_decoder);
  virtual ~CustomizedVideoDecoderProxy();
  int32_t InitDecode(const webrtc::VideoCodec* codec_settings,
                     int32_t number_of_cores) override;
  int32_t Decode(const EncodedImage& input,
                 bool missing_frames,
                 int64_t render_time_ms) override;
  int32_t RegisterDecodeCompleteCallback(
      DecodedImageCallback* callback) override;
  int32_t Release() override;
  const char* ImplementationName() const override;
 private:
  webrtc::VideoCodec codec_settings_;
  VideoCodecType codec_type_;
  DecodedImageCallback* decoded_image_callback_;
  VideoDecoderInterface* external_decoder_;
};

} // namespace base
} // namespace owt
#endif  // OWT_BASE_CUSTOMIZEDVIDEODECODERPROXY_H_
