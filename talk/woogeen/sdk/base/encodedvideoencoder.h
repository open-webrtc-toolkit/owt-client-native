/*
*
*
*/

#ifndef ENCODED_VIDEO_ENCODER_H_
#define ENCODED_VIDEO_ENCODER_H_

#include <vector>
#include "webrtc/video_encoder.h"

// This is provides a H264 encoder implementation using the WMF


class EncodedVideoEncoder : public webrtc::VideoEncoder {
public:
    EncodedVideoEncoder(webrtc::VideoCodecType type);

    ~EncodedVideoEncoder() override;

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
    webrtc::VideoCodecType codectype_;
};  // EncodedVideoEncoder
#endif  // ENCODED_VIDEO_ENCODER_H_
