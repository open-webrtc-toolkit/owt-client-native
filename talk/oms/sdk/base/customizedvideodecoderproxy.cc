/*
 * Intel License
 */
#include "talk/oms/sdk/base/customizedvideodecoderproxy.h"
#include "talk/oms/sdk/include/cpp/oms/base/commontypes.h"
#include "talk/oms/sdk/include/cpp/oms/base/videodecoderinterface.h"
namespace oms {
namespace base {
CustomizedVideoDecoderProxy::CustomizedVideoDecoderProxy(VideoCodecType type, VideoDecoderInterface* external_video_decoder)
  : codec_type_(type), decoded_image_callback_(nullptr), external_decoder_(external_video_decoder) {}
CustomizedVideoDecoderProxy::~CustomizedVideoDecoderProxy() {
  if (external_decoder_) {
    delete external_decoder_;
    external_decoder_ = nullptr;
  }
}
int32_t CustomizedVideoDecoderProxy::InitDecode(const webrtc::VideoCodec* codec_settings, int32_t number_of_cores) {
  RTC_CHECK(codec_settings->codecType == codec_type_)
    << "Unsupported codec type" << codec_settings->codecType << " for "
    << codec_type_;
  codec_settings_ = *codec_settings;
  if (external_decoder_) {
    if (codec_type_ == kVideoCodecH264 && external_decoder_->InitDecodeContext(VideoCodec::kH264)) {
      return WEBRTC_VIDEO_CODEC_OK;
    } else if (codec_type_ == kVideoCodecVP8 && external_decoder_->InitDecodeContext(VideoCodec::kVp8)) {
      return WEBRTC_VIDEO_CODEC_OK;
    }
    return WEBRTC_VIDEO_CODEC_ERROR;
  }
  return WEBRTC_VIDEO_CODEC_OK;
}
int32_t CustomizedVideoDecoderProxy::Decode(const EncodedImage& input_image,
                                            bool missing_frames,
                                            const CodecSpecificInfo* codec_specific_info,
                                            int64_t render_time_ms) {
  if (!decoded_image_callback_) {
    return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
  }
  if (!input_image._buffer || !input_image._length) {
    return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
  }
  // Check the codec information
  if (codec_specific_info && codec_specific_info->codecType != codec_type_) {
    return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
  }
  // TODO(chunbo): Fetch VideoFrame from the result of the decoder
  // Obtain the |video_frame| containing the decoded image.
  // decoded_image_callback_->Decoded(video_frame);
  if (external_decoder_) {
    std::unique_ptr<VideoEncodedFrame> frame(new VideoEncodedFrame{input_image._buffer, input_image._length, input_image._timeStamp, input_image._frameType == kVideoFrameKey});
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
  return "customized";
}
}
}
