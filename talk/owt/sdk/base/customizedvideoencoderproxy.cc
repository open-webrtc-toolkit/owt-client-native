// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include <string>
#include <vector>
#include "webrtc/api/video/video_frame.h"
#include "webrtc/modules/include/module_common_types.h"
#include "webrtc/modules/video_coding/include/video_codec_interface.h"
#include "webrtc/modules/video_coding/include/video_error_codes.h"
#include "webrtc/rtc_base/buffer.h"
#include "webrtc/rtc_base/checks.h"
#include "webrtc/rtc_base/logging.h"
#include "talk/owt/sdk/base/customizedencoderbufferhandle.h"
#include "talk/owt/sdk/base/customizedvideoencoderproxy.h"
#include "talk/owt/sdk/base/mediautils.h"
#include "talk/owt/sdk/base/nativehandlebuffer.h"
#include "talk/owt/sdk/include/cpp/owt/base/commontypes.h"

// H.264 start code length.
#define H264_SC_LENGTH 4
// Maximum allowed NALUs in one output frame.
#define MAX_NALUS_PERFRAME 32
using namespace rtc;
namespace owt {
namespace base {
static const uint8_t frame_number_sei_guid[16] = {
    0xef, 0xc8, 0xe7, 0xb0, 0x26, 0x26, 0x47, 0xfd,
    0x9d, 0xa3, 0x49, 0x4f, 0x60, 0xb8, 0x5b, 0xf0};

static const uint8_t cursor_data_sei_guid[16] = {
    0x2f, 0x69, 0xe7, 0xb0, 0x16, 0x56, 0x87, 0xfd,
    0x2d, 0x14, 0x26, 0x37, 0x14, 0x22, 0x23, 0x38};

CustomizedVideoEncoderProxy::CustomizedVideoEncoderProxy()
    : callback_(nullptr) {
  picture_id_ = 0;
}
CustomizedVideoEncoderProxy::~CustomizedVideoEncoderProxy() {}
int CustomizedVideoEncoderProxy::InitEncode(
    const webrtc::VideoCodec* codec_settings,
    int number_of_cores,
    size_t max_payload_size) {
  RTC_DCHECK(codec_settings);
  codec_type_ = codec_settings->codecType;
  width_ = codec_settings->width;
  height_ = codec_settings->height;
  bitrate_ = codec_settings->startBitrate * 1000;
  picture_id_ = static_cast<uint16_t>(rand()) & 0x7FFF;
  gof_.SetGofInfoVP9(TemporalStructureMode::kTemporalStructureMode1);
  gof_idx_ = 0;
  return WEBRTC_VIDEO_CODEC_OK;
}

int32_t CustomizedVideoEncoderProxy::Encode(
    const webrtc::VideoFrame& input_image,
    const std::vector<webrtc::VideoFrameType>* frame_types) {
  // Get the videoencoderinterface instance from the input video frame.
  CustomizedEncoderBufferHandle2* encoder_buffer_handle =
      reinterpret_cast<CustomizedEncoderBufferHandle2*>(
          static_cast<owt::base::EncodedFrameBuffer2*>(
              input_image.video_frame_buffer().get())
              ->native_handle());
  if (encoder_buffer_handle == nullptr ||
      encoder_buffer_handle->buffer_ == nullptr ||
      encoder_buffer_handle->buffer_length_ == 0) {
    RTC_LOG(LS_ERROR) << "Received invalid encoded frame.";
    return WEBRTC_VIDEO_CODEC_ERROR;
  }

  // Set encoder event callback object if not done already.
  if (encoder_event_callback_ == nullptr) {
    encoder_event_callback_ = encoder_buffer_handle->encoder_event_callback_;
  }

  // Check codec type before proceeding.
  {  // normal case.
#ifdef WEBRTC_USE_H265
    if (codec_type_ != webrtc::kVideoCodecH264 &&
        codec_type_ != webrtc::kVideoCodecVP8 &&
        codec_type_ != webrtc::kVideoCodecAV1 &&
        codec_type_ != webrtc::kVideoCodecVP9 &&
        codec_type_ != webrtc::kVideoCodecH265)
#else
    if (codec_type_ != webrtc::kVideoCodecH264 &&
        codec_type_ != webrtc::kVideoCodecVP8 &&
        codec_type_ != webrtc::kVideoCodecAV1 &&
        codec_type_ != webrtc::kVideoCodecVP9)
#endif
      return WEBRTC_VIDEO_CODEC_ERROR;
  }
  std::vector<uint8_t> buffer;
  bool request_key_frame = false;
  if (frame_types) {
    for (auto frame_type : *frame_types) {
      if (frame_type == webrtc::VideoFrameType::kVideoFrameKey) {
        request_key_frame = true;
        break;
      }
    }
  }

  if (encoder_event_callback_ != nullptr && request_key_frame) {
    encoder_event_callback_->RequestKeyFrame();
  }

  auto side_data_size =
      encoder_buffer_handle->meta_data_.encoded_image_sidedata_size();
  auto side_data_ptr =
      encoder_buffer_handle->meta_data_.encoded_image_sidedata_get();
  if (side_data_size > OWT_ENCODED_IMAGE_SIDE_DATA_SIZE_MAX) {
    return WEBRTC_VIDEO_CODEC_ERROR;
  }

  auto cursor_data_size = encoder_buffer_handle->meta_data_.cursor_data_size();
  auto cursor_data_ptr = encoder_buffer_handle->meta_data_.cursor_data_get();
  if (cursor_data_size > OWT_CURSOR_DATA_SIZE_MAX) {
    return WEBRTC_VIDEO_CODEC_ERROR;
  }

  // Construct the SEI. index 0-22/23 is the sei overhead, and last byte is
  // RBSP. Allocate the buffer to accommadate both AVC and HEVC. Construct
  // cursor data SEI and side-data SEI. Typically cursor data is larger than
  // 1KB, so the payload size should expand multiple bytes with 0xFF.
  int cursor_data_sei_payload__field_bytes = (cursor_data_size + 16 + 1) / 255;

  // 42 = 8(7 for hevc) + 16(side-data-guid) + 1(cursor sei payload type)
  // + 16(cursor_data-guid) + 1(rbsp trailing)
  std::unique_ptr<uint8_t[]> data(
      new uint8_t[encoder_buffer_handle->buffer_length_ + side_data_size +
                  cursor_data_sei_payload__field_bytes + cursor_data_size + 42]);
  uint8_t* data_ptr = data.get();
  uint32_t data_size =
      static_cast<uint32_t>(encoder_buffer_handle->buffer_length_);

  if ((codec_type_ != webrtc::kVideoCodecH264 &&
       codec_type_ != webrtc::kVideoCodecH265)) {
    if (side_data_size > 0)
      encoder_buffer_handle->meta_data_.encoded_image_sidedata_free();
    if (cursor_data_size > 0)
      encoder_buffer_handle->meta_data_.cursor_data_free();
  }

  int sei_idx = 0;

  if (codec_type_ == webrtc::kVideoCodecH264 &&
      ((side_data_ptr && side_data_size) ||
       (cursor_data_ptr && cursor_data_size))) {
    data_ptr[0] = data_ptr[1] = data_ptr[2] = 0;
    data_ptr[3] = 0x01;                 // start code: byte 0-3
    data_ptr[4] = 0x06;                 // NAL-type: SEI
    data_ptr[5] = 0x05;                 // userdata unregistered
    data_ptr[6] = 16 + side_data_size;  // payload size
    for (int i = 0; i < 16; i++) {
      data_ptr[i + 7] = frame_number_sei_guid[i];
    }
    if (side_data_size > 0 && side_data_ptr) {
      memcpy(data_ptr + 23, side_data_ptr, side_data_size);
      encoder_buffer_handle->meta_data_.encoded_image_sidedata_free();
    }
    sei_idx += 23 + side_data_size;

    // Done with side-data sei-message. Proceed with cursor-data sei-message.
    if (cursor_data_ptr && cursor_data_size) {
      data_ptr[sei_idx] = 0x05;  // userdata unregistered
      sei_idx++;
      if (cursor_data_sei_payload__field_bytes > 1) {
        for (int j = 0; j < cursor_data_sei_payload__field_bytes - 1; j++) {
          data_ptr[sei_idx] = 0xFF;
          sei_idx++;
        }
        data_ptr[sei_idx] = (cursor_data_size + 16) % 255;
        sei_idx++;
      } else {
        data_ptr[sei_idx] = cursor_data_size + 16;
        sei_idx++;
      }
      for (int i = 0; i < 16; i++) {
        data_ptr[sei_idx] = cursor_data_sei_guid[i];
        sei_idx++;
      }
      memcpy(data_ptr + sei_idx, cursor_data_ptr, cursor_data_size);
      sei_idx += cursor_data_size;
      encoder_buffer_handle->meta_data_.cursor_data_free();
    }

    data_ptr[sei_idx] = 0x80;
    sei_idx++;

    memcpy(data_ptr + sei_idx, encoder_buffer_handle->buffer_,
           encoder_buffer_handle->buffer_length_);
    data_size += sei_idx;
  } else if (codec_type_ == webrtc::kVideoCodecH265 &&
             ((side_data_ptr && side_data_size) ||
              (cursor_data_ptr && cursor_data_size))) {
    data_ptr[0] = data_ptr[1] = data_ptr[2] = 0;
    data_ptr[3] = 0x01;                 // start code: byte 0-3
    data_ptr[4] = 0x4e;                 // F: 0, nal_unit_type: prefix-SEI
    data_ptr[5] = 0x1;                  // layerID: 0; TID: 1
    data_ptr[6] = 0x05;                 // userdata unregistered
    data_ptr[7] = 16 + side_data_size;  // payload size
    for (int i = 0; i < 16; i++) {
      data_ptr[i + 8] = frame_number_sei_guid[i];
    }

    if (side_data_size > 0 && side_data_ptr) {
      memcpy(data_ptr + 24, side_data_ptr, side_data_size);
      encoder_buffer_handle->meta_data_.encoded_image_sidedata_free();
    }
    sei_idx += 24 + side_data_size;

    // Done with side-data sei-message. Proceed with cursor-data sei-message.
    if (cursor_data_ptr && cursor_data_size) {
      data_ptr[sei_idx] = 0x05;  // userdata unregistered
      sei_idx++;
      if (cursor_data_sei_payload__field_bytes > 1) {
        for (int j = 0; j < cursor_data_sei_payload__field_bytes - 1; j++) {
          data_ptr[sei_idx] = 0xFF;
          sei_idx++;
        }
        data_ptr[sei_idx] = (cursor_data_size + 16) % 255;
        sei_idx++;
      } else {
        data_ptr[sei_idx] = cursor_data_size + 16;
        sei_idx++;
      }
      for (int i = 0; i < 16; i++) {
        data_ptr[sei_idx] = cursor_data_sei_guid[i];
        sei_idx++;
      }
      memcpy(data_ptr + sei_idx, cursor_data_ptr, cursor_data_size);
      sei_idx += cursor_data_size;
      encoder_buffer_handle->meta_data_.cursor_data_free();
    }

    data_ptr[sei_idx] = 0x80;
    sei_idx++;

    memcpy(data_ptr + sei_idx, encoder_buffer_handle->buffer_,
           encoder_buffer_handle->buffer_length_);
    data_size += sei_idx;
  } else {
    memcpy(data_ptr, encoder_buffer_handle->buffer_,
           encoder_buffer_handle->buffer_length_);
  }

  webrtc::EncodedImage encoded_frame;
  encoded_frame.SetEncodedData(EncodedImageBuffer::Create(data_ptr, data_size));

  encoded_frame._encodedWidth = input_image.width();
  encoded_frame._encodedHeight = input_image.height();
  encoded_frame.capture_time_ms_ =
      /*input_image.render_time_ms()*/ encoder_buffer_handle->meta_data_
          .capture_timestamp;
  encoded_frame.SetTimestamp(input_image.timestamp());
  encoded_frame.playout_delay_.min_ms = 0;
  encoded_frame.playout_delay_.max_ms = 0;
  encoded_frame.timing_.encode_start_ms =
      encoder_buffer_handle->meta_data_.encoding_start;
  encoded_frame.timing_.encode_finish_ms =
      encoder_buffer_handle->meta_data_.encoding_end;
  encoded_frame.timing_.flags = webrtc::VideoSendTiming::kTriggeredByTimer;
  if (!update_ts_) {
    encoded_frame.capture_time_ms_ = last_capture_timestamp_;
    encoded_frame.SetTimestamp(last_timestamp_);
  } else {
    last_capture_timestamp_ = encoded_frame.capture_time_ms_;
    last_timestamp_ = encoded_frame.Timestamp();
  }

  if (encoder_buffer_handle->meta_data_.last_fragment)
    update_ts_ = true;
  else
    update_ts_ = false;

  // VP9 requires setting the frame type according to actual frame type.
  if (codec_type_ == webrtc::kVideoCodecVP9 && data_size > 2) {
    uint8_t au_key = 1;
    uint8_t first_byte = data_ptr[0], second_byte = data_ptr[1];
    uint8_t shift_bits = 4, profile = (first_byte >> shift_bits) & 0x3;
    shift_bits = (profile == 3) ? 2 : 3;
    uint8_t show_existing_frame = (first_byte >> shift_bits) & 0x1;
    if (profile == 3 && show_existing_frame) {
      au_key = (second_byte >> 6) & 0x1;
    } else if (profile == 3 && !show_existing_frame) {
      au_key = (first_byte >> 1) & 0x1;
    } else if (profile != 3 && show_existing_frame) {
      au_key = second_byte >> 7;
    } else {
      au_key = (first_byte >> 2) & 0x1;
    }
    encoded_frame._frameType = (au_key == 0)
                                  ? webrtc::VideoFrameType::kVideoFrameKey
                                  : webrtc::VideoFrameType::kVideoFrameDelta;
  }
  webrtc::CodecSpecificInfo info;
  memset(&info, 0, sizeof(info));
  info.codecType = codec_type_;
  if (codec_type_ == webrtc::kVideoCodecVP8) {
    info.codecSpecific.VP8.nonReference = false;
    info.codecSpecific.VP8.temporalIdx = webrtc::kNoTemporalIdx;
    info.codecSpecific.VP8.layerSync = false;
    info.codecSpecific.VP8.keyIdx = webrtc::kNoKeyIdx;
    picture_id_ = (picture_id_ + 1) & 0x7FFF;
  } else if (codec_type_ == webrtc::kVideoCodecVP9) {
    bool key_frame =
        encoded_frame._frameType == webrtc::VideoFrameType::kVideoFrameKey;
    if (key_frame) {
      gof_idx_ = 0;
    }
    info.codecSpecific.VP9.inter_pic_predicted = key_frame ? false : true;
    info.codecSpecific.VP9.flexible_mode = false;
    info.codecSpecific.VP9.ss_data_available = key_frame ? true : false;
    info.codecSpecific.VP9.temporal_idx = kNoTemporalIdx;
    //info.codecSpecific.VP9.spatial_idx = kNoSpatialIdx;
    info.codecSpecific.VP9.temporal_up_switch = true;
    info.codecSpecific.VP9.inter_layer_predicted = false;
    info.codecSpecific.VP9.gof_idx =
        static_cast<uint8_t>(gof_idx_++ % gof_.num_frames_in_gof);
    info.codecSpecific.VP9.num_spatial_layers = 1;
    info.codecSpecific.VP9.first_frame_in_picture = true;
    info.codecSpecific.VP9.end_of_picture = true;
    info.codecSpecific.VP9.spatial_layer_resolution_present = false;
    if (info.codecSpecific.VP9.ss_data_available) {
      info.codecSpecific.VP9.spatial_layer_resolution_present = true;
      info.codecSpecific.VP9.width[0] = width_;
      info.codecSpecific.VP9.height[0] = height_;
      info.codecSpecific.VP9.gof.CopyGofInfoVP9(gof_);
    }
  } else if (codec_type_ == webrtc::kVideoCodecH264) {
    int temporal_id = 0, priority_id = 0;
    bool is_idr = false;
    bool need_frame_marking = MediaUtils::GetH264TemporalInfo(
        data_ptr, data_size, temporal_id, priority_id, is_idr);
    if (need_frame_marking) {
      info.codecSpecific.H264.temporal_idx = temporal_id;
      info.codecSpecific.H264.base_layer_sync = (!is_idr && (temporal_id > 0));
    }
    info.codecSpecific.H264.idr_frame = is_idr;
    info.codecSpecific.H264.picture_id =
        encoder_buffer_handle->meta_data_.picture_id;
    info.codecSpecific.H264.last_fragment_in_frame =
        encoder_buffer_handle->meta_data_.last_fragment;
    encoded_frame._frameType = is_idr ? webrtc::VideoFrameType::kVideoFrameKey
                                     : webrtc::VideoFrameType::kVideoFrameDelta;
  } else if (codec_type_ == webrtc::kVideoCodecAV1) {
    encoded_frame._frameType = encoder_buffer_handle->meta_data_.is_keyframe
                                  ? webrtc::VideoFrameType::kVideoFrameKey
                                  : webrtc::VideoFrameType::kVideoFrameDelta;
    info.end_of_picture = encoder_buffer_handle->meta_data_.last_fragment;
  }
#ifdef WEBRTC_USE_H265
  else if (codec_type_ == webrtc::kVideoCodecH265) {
    info.codecSpecific.H265.picture_id =
        encoder_buffer_handle->meta_data_.picture_id;
    info.codecSpecific.H265.last_fragment_in_frame =
        encoder_buffer_handle->meta_data_.last_fragment;
    if (encoder_buffer_handle->meta_data_.frame_descriptor.active) {
      for (int i = 0; i < 5; i++) {
        if (encoder_buffer_handle->meta_data_.frame_descriptor.dependencies[i] >
            0)
          info.codecSpecific.H265.dependencies[i] =
              encoder_buffer_handle->meta_data_.frame_descriptor
                  .dependencies[i];
        else
          info.codecSpecific.H265.dependencies[i] = -1;
      }
      // TODO: DTIs not overlooked at present for HEVC.
    }
  }
#endif
  const auto result = callback_->OnEncodedImage(encoded_frame, &info);
  if (result.error != webrtc::EncodedImageCallback::Result::Error::OK) {
    RTC_LOG(LS_ERROR) << "Deliver encoded frame callback failed: "
                      << result.error;
    return WEBRTC_VIDEO_CODEC_ERROR;
  }
  return WEBRTC_VIDEO_CODEC_OK;
}
int CustomizedVideoEncoderProxy::RegisterEncodeCompleteCallback(
    webrtc::EncodedImageCallback* callback) {
  callback_ = callback;
  return WEBRTC_VIDEO_CODEC_OK;
}

void CustomizedVideoEncoderProxy::SetRates(
    const RateControlParameters& parameters) {
  if (parameters.framerate_fps < 1.0) {
    RTC_LOG(LS_WARNING) << "Unsupported framerate (must be >= 1.0";
    return;
  }
  if (encoder_event_callback_) {
    encoder_event_callback_->RequestRateUpdate(
        parameters.bandwidth_allocation.bps(), parameters.framerate_fps);
  }
}

void CustomizedVideoEncoderProxy::OnPacketLossRateUpdate(
    float packet_loss_rate) {
  // Currently not handled.
  return;
}

void CustomizedVideoEncoderProxy::OnRttUpdate(int64_t rtt_ms) {
  // Currently not handled.
  return;
}

void CustomizedVideoEncoderProxy::OnLossNotification(
    const LossNotification& loss_notification) {
  DependencyNotification notification;
  notification.timestamp_of_last_decodable =
      loss_notification.timestamp_of_last_decodable;
  notification.timestamp_of_last_received =
      loss_notification.timestamp_of_last_received;
  if (loss_notification.last_received_decodable.has_value()) {
    notification.last_received_decodable =
        loss_notification.last_received_decodable.value();
    notification.last_packet_not_received = false;
  } else {
    notification.last_packet_not_received = true;
  }
  if (loss_notification.dependencies_of_last_received_decodable.has_value()) {
    notification.last_frame_dependency_unknown = false;
    notification.dependencies_of_last_received_decodable =
        loss_notification.dependencies_of_last_received_decodable.value();
  } else {
    notification.last_frame_dependency_unknown = true;
  }
  if (encoder_event_callback_) {
    encoder_event_callback_->RequestLossNotification(notification);
  }
}

int CustomizedVideoEncoderProxy::Release() {
  callback_ = nullptr;
  return WEBRTC_VIDEO_CODEC_OK;
}
int32_t CustomizedVideoEncoderProxy::NextNaluPosition(uint8_t* buffer,
                                                      size_t buffer_size,
                                                      size_t* sc_length) {
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
      if (head[2] == 0x01) {
        *sc_length = 3;
        return (int32_t)(head - buffer);
      }
      head += 3;
      continue;
    }
    if (head[3] != 0x01) {  // got 000000xx
      head++;               // xx != 1, continue searching.
      continue;
    }
    *sc_length = 4;
    return (int32_t)(head - buffer);
  }
  return -1;
}

webrtc::VideoEncoder::EncoderInfo CustomizedVideoEncoderProxy::GetEncoderInfo()
    const {
  EncoderInfo info;
  info.supports_native_handle = true;
  info.is_hardware_accelerated = false;
  info.implementation_name = "OWTPassthroughEncoder";
  info.has_trusted_rate_controller = false;
  info.scaling_settings = VideoEncoder::ScalingSettings::kOff;
  return info;
}

std::unique_ptr<CustomizedVideoEncoderProxy>
CustomizedVideoEncoderProxy::Create() {
  return absl::make_unique<CustomizedVideoEncoderProxy>();
}

}  // namespace base
}  // namespace owt
