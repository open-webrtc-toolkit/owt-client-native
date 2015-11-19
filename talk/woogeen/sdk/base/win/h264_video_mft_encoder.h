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

#ifndef WEBRTC_MODULES_VIDEO_CODING_CODECS_H264_H264_VIDEO_MFT_ENCODER_H_
#define WEBRTC_MODULES_VIDEO_CODING_CODECS_H264_H264_VIDEO_MFT_ENCODER_H_

#include "webrtc/modules/video_coding/codecs/h264/include/h264.h"
#include <vector>

// This file provides a H264 encoder implementation using the WMF
// APIs.



class H264VideoMFTEncoder : public webrtc::H264Encoder {
public:
    H264VideoMFTEncoder();

    ~H264VideoMFTEncoder() override;

    int InitEncode(const webrtc::VideoCodec* codec_settings,
        int number_of_cores,
        size_t max_payload_size) override;

    int Encode(const webrtc::VideoFrame& input_image,
        const webrtc::CodecSpecificInfo* codec_specific_info,
        const std::vector<webrtc::VideoFrameType>* frame_types) override;

    int RegisterEncodeCompleteCallback(webrtc::EncodedImageCallback* callback) override;

    int SetChannelParameters(uint32_t packet_loss, int64_t rtt) override;

    int SetRates(uint32_t new_bitrate_kbit, uint32_t frame_rate) override;

    int Release() override;

private:

    webrtc::EncodedImageCallback* callback_;
    int32_t bitrate_;  // Bitrate in bits per second.
    int32_t width_;
    int32_t height_;
};  // H264VideoMFTEncoder
#endif  // WEBRTC_MODULES_VIDEO_CODING_CODECS_H264_H264_VIDEO_MFT_ENCODER_H_
