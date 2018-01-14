/*
* Intel License
*/
#include "webrtc/media/engine/webrtcvideoencoderfactory.h"
#include "webrtc/media/base/codec.h"
#include <vector>

#pragma once

class MSDKVideoEncoderFactory
    : public cricket::WebRtcVideoEncoderFactory{
public:
    MSDKVideoEncoderFactory();
    virtual ~MSDKVideoEncoderFactory();

    webrtc::VideoEncoder* CreateVideoEncoder(
        const cricket::VideoCodec& codec) override;
    const std::vector<cricket::VideoCodec>& supported_codecs() const override;
    void DestroyVideoEncoder(webrtc::VideoEncoder* encoder) override;

private:
 std::vector<cricket::VideoCodec> supported_codecs_;
};
