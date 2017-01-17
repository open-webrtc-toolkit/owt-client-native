/*
 * Intel License
 */

#ifndef WOOGEEN_BASE_CUSTOMIZEDVIDEODECODERFACTORY_H_
#define WOOGEEN_BASE_CUSTOMIZEDVIDEODECODERFACTORY_H_

#include "webrtc/media/engine/webrtcvideodecoderfactory.h"
#include "talk/woogeen/sdk/base/customizedvideodecoderproxy.h"
#include "talk/woogeen/sdk/include/cpp/woogeen/base/videodecoderinterface.h"

#pragma once

// Declaration of customized based decoder factory.
class CustomizedVideoDecoderFactory
    : public cricket::WebRtcVideoDecoderFactory {
public:
    CustomizedVideoDecoderFactory(std::unique_ptr<woogeen::base::VideoDecoderInterface> external_decoder);
    virtual ~CustomizedVideoDecoderFactory();

    // WebRtcVideoDecoderFactory implementation.
    webrtc::VideoDecoder* CreateVideoDecoder(webrtc::VideoCodecType type) override;

    void DestroyVideoDecoder(webrtc::VideoDecoder* decoder) override;

private:
    std::vector<webrtc::VideoCodecType> supported_codec_types_;
    std::unique_ptr<woogeen::base::VideoDecoderInterface> external_decoder_;
};

#endif  // WOOGEEN_BASE_CUSTOMIZEDVIDEODECODERFACTORY_H_