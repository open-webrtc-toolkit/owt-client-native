/*
*
*
*
*/
#ifndef ENCODED_VIDEO_ENCODER_FACTORY_H_
#define ENCODED_VIDEO_ENCODER_FACTORY_H_

#include <vector>
#include "talk/media/webrtc/webrtcvideoencoderfactory.h"

class EncodedVideoEncoderFactory
    : public cricket::WebRtcVideoEncoderFactory{
public:
    EncodedVideoEncoderFactory();
    virtual ~EncodedVideoEncoderFactory();

    webrtc::VideoEncoder* CreateVideoEncoder(webrtc::VideoCodecType type) override;
    const std::vector<VideoCodec>& codecs() const override;
    void DestroyVideoEncoder(webrtc::VideoEncoder* encoder) override;

private:
    std::vector<VideoCodec> supported_codecs_;
};

#endif  // ENCODED_VIDEO_ENCODER_FACTORY_H_
