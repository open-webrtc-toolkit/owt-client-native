// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "talk/owt/sdk/base/customizedvideodecoderproxy.h"
#include "talk/owt/sdk/include/cpp/owt/base/commontypes.h"
#include "talk/owt/sdk/include/cpp/owt/base/videodecoderinterface.h"
namespace owt {
namespace base {

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
  if (external_decoder_) {
    if (codec_type_ == kVideoCodecH264 &&
        external_decoder_->InitDecodeContext(VideoCodec::kH264)) {
      return true;
#ifdef WEBRTC_USE_H265
    } else if (codec_type_ == kVideoCodecH265 &&
               external_decoder_->InitDecodeContext(VideoCodec::kH265)) {
      return true;
#endif
    } else if (codec_type_ == kVideoCodecVP8 &&
               external_decoder_->InitDecodeContext(VideoCodec::kVp8)) {
      return true;
    } else if (codec_type_ == kVideoCodecVP9 &&
               external_decoder_->InitDecodeContext(VideoCodec::kVp9)) {
      return true;
    } else if (codec_type_ == kVideoCodecAV1 &&
               external_decoder_->InitDecodeContext(VideoCodec::kAv1)) {
      return true;
    }
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
