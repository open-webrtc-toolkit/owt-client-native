/*
*
*/

//
//V4L2VideoDecoderFactory implemenation
//
#include <iostream>
#include "talk/woogeen/sdk/base/linux/v4l2videodecoderfactory.h"
V4L2VideoDecoderFactory::V4L2VideoDecoderFactory() {
    supported_codec_types_.clear();
    supported_codec_types_.push_back(webrtc::kVideoCodecH264);
    supported_codec_types_.push_back(webrtc::kVideoCodecVP8);
}


V4L2VideoDecoderFactory::~V4L2VideoDecoderFactory() {
}

webrtc::VideoDecoder* V4L2VideoDecoderFactory::CreateVideoDecoder(webrtc::VideoCodecType type) {
    if (supported_codec_types_.empty()) {
        return NULL;
    }
    for (std::vector<webrtc::VideoCodecType>::const_iterator it =
        supported_codec_types_.begin(); it != supported_codec_types_.end();
        ++it) {
        if (*it == type) {
            return new webrtc::V4L2Decoder();
        }
    }
    return NULL;
}

void V4L2VideoDecoderFactory::DestroyVideoDecoder(
    webrtc::VideoDecoder* decoder) {
    delete decoder;
}
