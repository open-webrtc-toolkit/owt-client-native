/*
 * Intel License
 */
#include "webrtc/media/engine/webrtcvideodecoderfactory.h"
#pragma once
// Declaration of MSDK based decoder factory.
class MSDKVideoDecoderFactory
    : public cricket::WebRtcVideoDecoderFactory {
public:
 MSDKVideoDecoderFactory();
 virtual ~MSDKVideoDecoderFactory();

 // WebRtcVideoDecoderFactory implementation.
 webrtc::VideoDecoder* CreateVideoDecoder(webrtc::VideoCodecType type) override;

 void DestroyVideoDecoder(webrtc::VideoDecoder* decoder) override;

private:
    std::vector<webrtc::VideoCodecType> supported_codec_types_;
};
