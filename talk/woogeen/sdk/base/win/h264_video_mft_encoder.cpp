/*
*  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*
*/

#include "talk/woogeen/sdk/base/win/h264_video_mft_encoder.h"
#include <string>
#include <vector>
#include "webrtc/base/checks.h"
#include "webrtc/base/logging.h"
#include "webrtc/base/scoped_ptr.h"



H264VideoMFTEncoder::H264VideoMFTEncoder()
    : callback_(nullptr) {
}

H264VideoMFTEncoder::~H264VideoMFTEncoder() {
}

int H264VideoMFTEncoder::InitEncode(const webrtc::VideoCodec* codec_settings,
    int number_of_cores,
    size_t max_payload_size) {
    RTC_DCHECK(codec_settings);
    RTC_DCHECK_EQ(codec_settings->codecType, webrtc::kVideoCodecH264);
    // TODO(tkchin): We may need to enforce width/height dimension restrictions
    // to match what the encoder supports.
    width_ = codec_settings->width;
    height_ = codec_settings->height;
    // We can only set average bitrate on the HW encoder.
    bitrate_ = codec_settings->startBitrate * 1000;

    // TODO(tkchin): Try setting payload size via
    // kVTCompressionPropertyKey_MaxH264SliceBytes.

    return WEBRTC_VIDEO_CODEC_OK;
}

int H264VideoMFTEncoder::Encode(
    const webrtc::VideoFrame& input_image,
    const webrtc::CodecSpecificInfo* codec_specific_info,
    const std::vector<webrtc::VideoFrameType>* frame_types) {
    return WEBRTC_VIDEO_CODEC_OK;
}

int H264VideoMFTEncoder::RegisterEncodeCompleteCallback(
    webrtc::EncodedImageCallback* callback) {
    callback_ = callback;
    return WEBRTC_VIDEO_CODEC_OK;
}

int H264VideoMFTEncoder::SetChannelParameters(uint32_t packet_loss,
    int64_t rtt) {
    // Encoder doesn't know anything about packet loss or rtt so just return.
    return WEBRTC_VIDEO_CODEC_OK;
}

int H264VideoMFTEncoder::SetRates(uint32_t new_bitrate_kbit,
    uint32_t frame_rate) {
    return WEBRTC_VIDEO_CODEC_OK;
}

int H264VideoMFTEncoder::Release() {
    callback_ = nullptr;
    // Need to reset to that the session is invalidated and won't use the
    // callback anymore.
    return WEBRTC_VIDEO_CODEC_OK;
}




