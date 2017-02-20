/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "talk/woogeen/sdk/base/linux/v4l2videodecoder.h"

namespace webrtc {
V4L2Decoder::V4L2Decoder() : decoded_image_callback_(nullptr) {}

int32_t V4L2Decoder::InitDecode(const VideoCodec* config,
                                int32_t number_of_cores) {
  config_ = *config;

  return WEBRTC_VIDEO_CODEC_OK;
}

int32_t V4L2Decoder::Decode(const EncodedImage& input_image,
                            bool missing_frames,
                            const RTPFragmentationHeader* fragmentation,
                            const CodecSpecificInfo* codec_specific_info,
                            int64_t render_time_ms) {
  if (!decoded_image_callback_) {
    return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
  }

  if (!input_image._buffer || !input_image._length) {
    return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
  }

  // TODO(chunbo): Check the codec information
  // if (codec_specific_info &&
  //     codec_specific_info->codecType != kVideoCodecH264) {
  //   return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
  // }

  // TODO(chunbo): Fetch VideoFrame from the result of the decoder
  // Obtain the |video_frame| containing the decoded image.
  // decoded_image_callback_->Decoded(video_frame);

  return WEBRTC_VIDEO_CODEC_OK;
}

int32_t V4L2Decoder::RegisterDecodeCompleteCallback(
    DecodedImageCallback* callback) {
  decoded_image_callback_ = callback;
  return WEBRTC_VIDEO_CODEC_OK;
}

int32_t V4L2Decoder::Release() {
  return WEBRTC_VIDEO_CODEC_OK;
}

const char* V4L2Decoder::ImplementationName() const {
  return "woogeen";
}

int32_t V4L2H264Decoder::Decode(const EncodedImage& input,
                                bool missing_frames,
                                const RTPFragmentationHeader* fragmentation,
                                const CodecSpecificInfo* codec_specific_info,
                                int64_t render_time_ms) {
  uint8_t value = 0;
  for (size_t i = 0; i < input._length; ++i) {
    uint8_t kStartCode[] = {0, 0, 0, 1};
    if (i < input._length - sizeof(kStartCode) &&
        !memcmp(&input._buffer[i], kStartCode, sizeof(kStartCode))) {
      i += sizeof(kStartCode) + 1;  // Skip start code and NAL header.
    }
    if (input._buffer[i] != value) {
      return -1;
    }
    ++value;
  }
  return V4L2Decoder::Decode(input,
                             missing_frames,
                             fragmentation,
                             codec_specific_info,
                             render_time_ms);
}

}
