// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_ENCODEDVIDEOENCODER_H_
#define OWT_BASE_ENCODEDVIDEOENCODER_H_
#include <vector>
#include "webrtc/api/video_codecs/video_encoder.h"
#include "talk/owt/sdk/include/cpp/owt/base/videoencoderinterface.h"
namespace owt {
namespace base {
class CustomizedVideoEncoderProxy : public webrtc::VideoEncoder {
 public:
  CustomizedVideoEncoderProxy(webrtc::VideoCodecType type);
  virtual ~CustomizedVideoEncoderProxy();
  int InitEncode(const webrtc::VideoCodec* codec_settings,
                 int number_of_cores,
                 size_t max_payload_size) override;
  int Encode(const webrtc::VideoFrame& input_image,
             const webrtc::CodecSpecificInfo* codec_specific_info,
             const std::vector<webrtc::FrameType>* frame_types) override;
  int RegisterEncodeCompleteCallback(
      webrtc::EncodedImageCallback* callback) override;
  int SetChannelParameters(uint32_t packet_loss, int64_t rtt) override;
  int SetRates(uint32_t new_bitrate_kbit, uint32_t frame_rate) override;
  bool SupportsNativeHandle() const override;
  int Release() override;
 private:
  // Search for H.264 start codes.
  int32_t NextNaluPosition(uint8_t* buffer, size_t buffer_size, size_t* sc_length);
  webrtc::EncodedImageCallback* callback_;
  int32_t bitrate_;  // Bitrate in bits per second.
  int32_t width_;
  int32_t height_;
  // int count_;
  webrtc::VideoCodecType codec_type_;
  uint16_t picture_id_;
  VideoEncoderInterface* external_encoder_;
};
}
}
#endif  // OWT_BASE_ENCODEDVIDEOENCODER_H_
