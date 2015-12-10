/*
*
*
*/

#include "talk/woogeen/sdk/base/encodedvideoencoderfactory.h"
#include "talk/woogeen/sdk/base/encodedvideoencoder.h"

#define MAX_VIDEO_WIDTH 1080
#define MAX_VIDEO_HEIGHT 720
#define MAX_VIDEO_FPS 30

EncodedVideoEncoderFactory::EncodedVideoEncoderFactory(){
    supported_codecs_.clear();
    supported_codecs_.push_back(VideoCodec(webrtc::kVideoCodecVP8, "VP8", MAX_VIDEO_WIDTH, MAX_VIDEO_HEIGHT, MAX_VIDEO_FPS));
    supported_codecs_.push_back(VideoCodec(webrtc::kVideoCodecH264, "H264", MAX_VIDEO_WIDTH, MAX_VIDEO_HEIGHT, MAX_VIDEO_FPS));
}

EncodedVideoEncoderFactory::~EncodedVideoEncoderFactory(){}

webrtc::VideoEncoder* EncodedVideoEncoderFactory::CreateVideoEncoder(webrtc::VideoCodecType type){
    if (supported_codecs_.empty()){
        return NULL;
    }
    for (std::vector<VideoCodec>::const_iterator it = supported_codecs_.begin(); it != supported_codecs_.end(); ++it){
        if (it->type == type){
            return new EncodedVideoEncoder(type);
        }
    }
    return NULL;
}

const std::vector<EncodedVideoEncoderFactory::VideoCodec>& EncodedVideoEncoderFactory::codecs() const{
    return supported_codecs_;
}

void EncodedVideoEncoderFactory::DestroyVideoEncoder(webrtc::VideoEncoder* encoder){
    delete encoder;
}
