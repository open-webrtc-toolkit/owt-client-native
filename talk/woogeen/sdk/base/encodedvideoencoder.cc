/*
*
*
*/

#include <string>
#include <vector>
#include "webrtc/base/checks.h"
#include "webrtc/base/logging.h"
#include "webrtc/base/scoped_ptr.h"
#include "webrtc/base/buffer.h"
#include "webrtc/modules/video_coding/codecs/interface/video_error_codes.h"
#include "talk/woogeen/sdk/base/encodedvideoencoder.h"

EncodedVideoEncoder::EncodedVideoEncoder(webrtc::VideoCodecType type)
    : callback_(nullptr) {
    codectype_ = type;
}

EncodedVideoEncoder::~EncodedVideoEncoder() {
}

int EncodedVideoEncoder::InitEncode(const webrtc::VideoCodec* codec_settings,
    int number_of_cores,
    size_t max_payload_size) {
    RTC_DCHECK(codec_settings);
    RTC_DCHECK_EQ(codec_settings->codecType, codectype_);

    width_ = codec_settings->width;
    height_ = codec_settings->height;
    bitrate_ = codec_settings->startBitrate * 1000;

    return WEBRTC_VIDEO_CODEC_OK;
}

int EncodedVideoEncoder::Encode(
    const webrtc::VideoFrame& input_image,
    const webrtc::CodecSpecificInfo* codec_specific_info,
    const std::vector<webrtc::VideoFrameType>* frame_types) {

    // ToDo: Jianlin --- the encoded from input_image will be write to buffer late
    rtc::scoped_ptr<rtc::Buffer> buffer(new rtc::Buffer());
    webrtc::EncodedImage encodedframe(buffer->data(), buffer->size(), buffer->size());
    encodedframe._encodedWidth = input_image.width();
    encodedframe._encodedHeight = input_image.height();
    //encodedframe._completeFrame = true;

    // Check if we need a keyframe.
    bool is_keyframe = false;
    if (frame_types) {
      for (auto frame_type : *frame_types) {
        if (frame_type == webrtc::kKeyFrame) {
          is_keyframe = true;
          break;
        }
      }
    }

    encodedframe._frameType = is_keyframe ? webrtc::kKeyFrame : webrtc::kDeltaFrame;
    encodedframe.capture_time_ms_ = input_image.render_time_ms();
    encodedframe._timeStamp = input_image.timestamp();

    //int result = callback_->Encoded(encodedframe, &(encode_params->codec_specific_info), header.get());
    int result = callback_->Encoded(encodedframe, NULL, NULL);
    if (result != 0) {
      LOG(LS_ERROR) << "Deliver encoded frame callback failed: " << result;
      return WEBRTC_VIDEO_CODEC_ERROR;
    }
    return WEBRTC_VIDEO_CODEC_OK;
}

int EncodedVideoEncoder::RegisterEncodeCompleteCallback(
    webrtc::EncodedImageCallback* callback) {
    callback_ = callback;
    return WEBRTC_VIDEO_CODEC_OK;
}

int EncodedVideoEncoder::SetChannelParameters(uint32_t packet_loss,
    int64_t rtt) {
    return WEBRTC_VIDEO_CODEC_OK;
}

int EncodedVideoEncoder::SetRates(uint32_t new_bitrate_kbit,
    uint32_t frame_rate) {
    bitrate_ = new_bitrate_kbit * 1000;
    return WEBRTC_VIDEO_CODEC_OK;
}

int EncodedVideoEncoder::Release() {
    callback_ = nullptr;
    return WEBRTC_VIDEO_CODEC_OK;
}




