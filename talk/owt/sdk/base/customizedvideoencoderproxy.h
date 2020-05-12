// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_ENCODEDVIDEOENCODER_H_
#define OWT_BASE_ENCODEDVIDEOENCODER_H_
#include <vector>
#include "webrtc/api/video_codecs/video_encoder.h"
#include "webrtc/media/base/codec.h"
#include "talk/owt/sdk/include/cpp/owt/base/videoencoderinterface.h"

namespace owt {
namespace base {
class CustomizedVideoEncoderProxy : public webrtc::VideoEncoder {
 public:
  static std::unique_ptr<CustomizedVideoEncoderProxy> Create();
  CustomizedVideoEncoderProxy();
  virtual ~CustomizedVideoEncoderProxy();
  int InitEncode(const webrtc::VideoCodec* codec_settings,
                 int number_of_cores,
                 size_t max_payload_size) override;
  int32_t Encode(const webrtc::VideoFrame& input_image,
             const std::vector<webrtc::VideoFrameType>* frame_types) override;
  int RegisterEncodeCompleteCallback(
      webrtc::EncodedImageCallback* callback) override;
  void SetRates(const RateControlParameters& parameters) override;
  void OnPacketLossRateUpdate(float packet_loss_rate) override;
  void OnRttUpdate(int64_t rtt_ms) override;
  void OnLossNotification(const LossNotification& loss_notification) override;
  EncoderInfo GetEncoderInfo() const override;
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
  EncoderEventCallback* encoder_event_callback_ = nullptr;
  uint32_t last_timestamp_;
  uint64_t last_capture_timestamp_;
  bool update_ts_ = true;
  uint8_t gof_idx_;
  webrtc::GofInfoVP9 gof_;
};
}
}
#endif  // OWT_BASE_ENCODEDVIDEOENCODER_H_
