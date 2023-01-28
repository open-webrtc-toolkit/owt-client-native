// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "talk/owt/sdk/base/customizedvideodecoderproxy.h"
#include "talk/owt/sdk/include/cpp/owt/base/commontypes.h"
#include "talk/owt/sdk/include/cpp/owt/base/videodecoderinterface.h"
namespace owt {
namespace base {

std::unordered_map<webrtc::VideoCodecType, owt::base::VideoCodec>
    video_codec_map = {
        {webrtc::VideoCodecType::kVideoCodecH264, owt::base::VideoCodec::kH264},
#ifdef WEBRTC_USE_H265
        {webrtc::VideoCodecType::kVideoCodecH265, owt::base::VideoCodec::kH265},
#endif
        {webrtc::VideoCodecType::kVideoCodecVP8, owt::base::VideoCodec::kVp8},
        {webrtc::VideoCodecType::kVideoCodecVP9, owt::base::VideoCodec::kVp9},
        {webrtc::VideoCodecType::kVideoCodecAV1, owt::base::VideoCodec::kAv1},
};

CustomizedVideoDecoderProxy::CustomizedVideoDecoderProxy(VideoCodecType type,
  VideoDecoderInterface* external_video_decoder)
  : codec_type_(type),decoded_image_callback_(nullptr), external_decoder_(external_video_decoder) {}

CustomizedVideoDecoderProxy::~CustomizedVideoDecoderProxy() {
  if (external_decoder_) {
    delete external_decoder_;
    external_decoder_ = nullptr;
  }
}

bool CustomizedVideoDecoderProxy::Configure(const Settings& codec_settings) {
  RTC_CHECK(codec_settings.codec_type() == codec_type_)
      << "Unsupported codec type" << codec_settings.codec_type() << " for "
      << codec_type_;
  RTC_DCHECK(video_codec_map.contains(codec_type_));
  if (!external_decoder_ ||
      !external_decoder_->InitDecodeContext(video_codec_map[codec_type_])) {
    return false;
  }
  return true;
}

int32_t CustomizedVideoDecoderProxy::Decode(const EncodedImage& input_image,
                                            bool missing_frames,
                                            int64_t render_time_ms) {
  if (!decoded_image_callback_) {
    return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
  }
  if (!input_image.data()|| !input_image.size()) {
    return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
  }

  // TODO(chunbo): Fetch VideoFrame from the result of the decoder
  // Obtain the |video_frame| containing the decoded image.
  // decoded_image_callback_->Decoded(video_frame);
  if (external_decoder_) {
    std::unique_ptr<VideoEncodedFrame> frame(new VideoEncodedFrame{input_image.data(), input_image.size(), input_image.Timestamp(), input_image._frameType == webrtc::VideoFrameType::kVideoFrameKey});
    if (external_decoder_->OnEncodedFrame(std::move(frame))) {
      return WEBRTC_VIDEO_CODEC_OK;
    }
    return WEBRTC_VIDEO_CODEC_ERROR;
  }
  return WEBRTC_VIDEO_CODEC_OK;
}

int32_t CustomizedVideoDecoderProxy::RegisterDecodeCompleteCallback(
    DecodedImageCallback* callback) {
  decoded_image_callback_ = callback;
  return WEBRTC_VIDEO_CODEC_OK;
}

int32_t CustomizedVideoDecoderProxy::Release() {
  if (external_decoder_) {
    if (external_decoder_->Release()) {
      return WEBRTC_VIDEO_CODEC_OK;
    }
    return WEBRTC_VIDEO_CODEC_ERROR;
  }
  return WEBRTC_VIDEO_CODEC_OK;
}

const char* CustomizedVideoDecoderProxy::ImplementationName() const {
  return "CustomizedDecoder";
}

std::unique_ptr<CustomizedVideoDecoderProxy>
CustomizedVideoDecoderProxy::Create(VideoCodecType type,
    VideoDecoderInterface* external_video_decoder) {
  return absl::make_unique<CustomizedVideoDecoderProxy>(type, external_video_decoder);
}
}
}
