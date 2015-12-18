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
V4L2Decoder::V4L2Decoder() : callback_(NULL) {}

int32_t V4L2Decoder::InitDecode(const VideoCodec* config,
                                int32_t number_of_cores) {
  config_ = *config;
  size_t width = config->width;
  size_t height = config->height;
  frame_.CreateEmptyFrame(static_cast<int>(width),
                          static_cast<int>(height),
                          static_cast<int>(width),
                          static_cast<int>((width + 1) / 2),
                          static_cast<int>((width + 1) / 2));
  return WEBRTC_VIDEO_CODEC_OK;
}

int32_t V4L2Decoder::Decode(const EncodedImage& input,
                            bool missing_frames,
                            const RTPFragmentationHeader* fragmentation,
                            const CodecSpecificInfo* codec_specific_info,
                            int64_t render_time_ms) {
  frame_.set_timestamp(input._timeStamp);
  frame_.set_ntp_time_ms(input.ntp_time_ms_);
  frame_.set_render_time_ms(render_time_ms);

  callback_->Decoded(frame_);

  return WEBRTC_VIDEO_CODEC_OK;
}

int32_t V4L2Decoder::RegisterDecodeCompleteCallback(
    DecodedImageCallback* callback) {
  callback_ = callback;
  return WEBRTC_VIDEO_CODEC_OK;
}

int32_t V4L2Decoder::Release() {
  return WEBRTC_VIDEO_CODEC_OK;
}
int32_t V4L2Decoder::Reset() {
  return WEBRTC_VIDEO_CODEC_OK;
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
