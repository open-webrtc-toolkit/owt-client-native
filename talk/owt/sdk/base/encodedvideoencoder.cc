// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include <string>
#include <vector>
#include "talk/owt/sdk/base/encodedvideoencoder.h"
#include "webrtc/rtc_base/checks.h"
#include "webrtc/rtc_base/logging.h"
#include "webrtc/rtc_base/buffer.h"
#include "webrtc/common_video/libyuv/include/webrtc_libyuv.h"
#include "webrtc/modules/video_coding/include/video_error_codes.h"
#include "webrtc/modules/video_coding/include/video_codec_interface.h"
#include "webrtc/modules/include/module_common_types.h"
#include "webrtc/api/video/video_frame.h"
// H.264 start code length.
#define H264_SC_LENGTH 4
// Maximum allowed NALUs in one output frame.
#define MAX_NALUS_PERFRAME 32
EncodedVideoEncoder::EncodedVideoEncoder(webrtc::VideoCodecType type)
    : callback_(nullptr) {
  codecType_ = type;
  picture_id_ = 0;
}
EncodedVideoEncoder::~EncodedVideoEncoder() {}
int EncodedVideoEncoder::InitEncode(const webrtc::VideoCodec* codec_settings,
                                    int number_of_cores,
                                    size_t max_payload_size) {
  RTC_DCHECK(codec_settings);
  RTC_DCHECK_EQ(codec_settings->codecType, codecType_);
  width_ = codec_settings->width;
  height_ = codec_settings->height;
  bitrate_ = codec_settings->startBitrate * 1000;
  picture_id_ = static_cast<uint16_t>(rand()) & 0x7FFF;
  return WEBRTC_VIDEO_CODEC_OK;
}
int EncodedVideoEncoder::Encode(
    const webrtc::VideoFrame& input_image,
    const webrtc::CodecSpecificInfo* codec_specific_info,
    const std::vector<webrtc::FrameType>* frame_types) {
  rtc::scoped_refptr<webrtc::VideoFrameBuffer> video_frame_buffer(
      input_image.video_frame_buffer());
  if (video_frame_buffer == nullptr) {
    return WEBRTC_VIDEO_CODEC_ERROR;
  }
  uint8_t* data = static_cast<uint8_t*>(video_frame_buffer->native_handle());
  int data_size = webrtc::CalcBufferSize(webrtc::kI420, input_image.width(),
                                         input_image.height());
  if (data == nullptr) {
    return WEBRTC_VIDEO_CODEC_ERROR;
  }
  /*int data_size;
  if(fread(&data_size, 1, sizeof(int), fd) != sizeof(int)) {
    fseek(fd, 0, SEEK_SET);
    fread(&data_size, sizeof(int), 1, fd);
  }
  uint8* data = new uint8[data_size];
  fread(data, 1, data_size, fd);*/
  webrtc::EncodedImage encodedframe(data, data_size, data_size);
  encodedframe._encodedWidth = input_image.width();
  encodedframe._encodedHeight = input_image.height();
  encodedframe._completeFrame = true;
  // Check if we need a keyframe.
  /*bool is_keyframe = false;
  if (frame_types) {
    for (auto frame_type : *frame_types) {
      if (frame_type == webrtc::kKeyFrame) {
        is_keyframe = true;
        break;
      }
    }
  }
  encodedframe._frameType = is_keyframe ? webrtc::kKeyFrame :
  webrtc::kDeltaFrame;*/
  encodedframe.capture_time_ms_ = input_image.render_time_ms();
  encodedframe._timeStamp = input_image.timestamp();
  webrtc::CodecSpecificInfo info;
  memset(&info, 0, sizeof(info));
  info.codecType = codecType_;
  if (codecType_ == webrtc::kVideoCodecVP8) {
    info.codecSpecific.VP8.pictureId = picture_id_;
    info.codecSpecific.VP8.nonReference = false;
    info.codecSpecific.VP8.simulcastIdx = 0;
    info.codecSpecific.VP8.temporalIdx = webrtc::kNoTemporalIdx;
    info.codecSpecific.VP8.layerSync = false;
    info.codecSpecific.VP8.tl0PicIdx = webrtc::kNoTl0PicIdx;
    info.codecSpecific.VP8.keyIdx = webrtc::kNoKeyIdx;
    picture_id_ = (picture_id_ + 1) & 0x7FFF;
  }
  // Generate a header describing a single fragment.
  webrtc::RTPFragmentationHeader header;
  memset(&header, 0, sizeof(header));
  if (codecType_ == webrtc::kVideoCodecVP8) {
    header.VerifyAndAllocateFragmentationHeader(1);
    header.fragmentationOffset[0] = 0;
    header.fragmentationLength[0] = encodedframe._length;
    header.fragmentationPlType[0] = 0;
    header.fragmentationTimeDiff[0] = 0;
  } else if (codecType_ == webrtc::kVideoCodecH264) {
    // For H.264 search for start codes.
    int32_t scPositions[MAX_NALUS_PERFRAME + 1] = {};
    int32_t scPositionsLength = 0;
    int32_t scPosition = 0;
    while (scPositionsLength < MAX_NALUS_PERFRAME) {
      int32_t naluPosition =
          NextNaluPosition(data + scPosition, data_size - scPosition);
      if (naluPosition < 0) {
        break;
      }
      scPosition += naluPosition;
      scPositions[scPositionsLength++] = scPosition;
      scPosition += H264_SC_LENGTH;
    }
    if (scPositionsLength == 0) {
      LOG(LS_ERROR) << "Start code is not found for H264 codec!";
      return WEBRTC_VIDEO_CODEC_ERROR;
    }
    scPositions[scPositionsLength] = data_size;
    header.VerifyAndAllocateFragmentationHeader(scPositionsLength);
    for (int i = 0; i < scPositionsLength; i++) {
      header.fragmentationOffset[i] = scPositions[i] + H264_SC_LENGTH;
      header.fragmentationLength[i] =
          scPositions[i + 1] - header.fragmentationOffset[i];
      header.fragmentationPlType[i] = 0;
      header.fragmentationTimeDiff[i] = 0;
    }
  }
  const auto result = callback_->OnEncodedImage(encodedframe, &info, &header);
  if (result.error != webrtc::EncodedImageCallback::Result::Error::OK) {
    LOG(LS_ERROR) << "Deliver encoded frame callback failed: " << result.error;
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
bool EncodedVideoEncoder::SupportsNativeHandle() const {
  return true;
}
int EncodedVideoEncoder::Release() {
  callback_ = nullptr;
  return WEBRTC_VIDEO_CODEC_OK;
}
int32_t EncodedVideoEncoder::NextNaluPosition(uint8_t* buffer,
                                              size_t buffer_size) {
  if (buffer_size < H264_SC_LENGTH) {
    return -1;
  }
  uint8_t* head = buffer;
  // Set end buffer pointer to 4 bytes before actual buffer end so we can
  // access head[1], head[2] and head[3] in a loop without buffer overrun.
  uint8_t* end = buffer + buffer_size - H264_SC_LENGTH;
  while (head < end) {
    if (head[0]) {
      head++;
      continue;
    }
    if (head[1]) {  // got 00xx
      head += 2;
      continue;
    }
    if (head[2]) {  // got 0000xx
      head += 3;
      continue;
    }
    if (head[3] != 0x01) {  // got 000000xx
      head++;               // xx != 1, continue searching.
      continue;
    }
    return (int32_t)(head - buffer);
  }
  return -1;
}
