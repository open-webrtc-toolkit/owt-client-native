/*
 * Intel License
 */

#ifndef ICS_BASE_CUSTOMIZEDVIDEODECODERFACTORY_H_
#define ICS_BASE_CUSTOMIZEDVIDEODECODERFACTORY_H_

#include "webrtc/media/engine/webrtcvideodecoderfactory.h"
#include "talk/ics/sdk/base/customizedvideodecoderproxy.h"
#include "talk/ics/sdk/include/cpp/ics/base/videodecoderinterface.h"

#pragma once

// Declaration of customized based decoder factory.
class CustomizedVideoDecoderFactory
    : public cricket::WebRtcVideoDecoderFactory {
public:
    CustomizedVideoDecoderFactory(std::unique_ptr<ics::base::VideoDecoderInterface> external_decoder);
    virtual ~CustomizedVideoDecoderFactory();

    // WebRtcVideoDecoderFactory implementation.
    webrtc::VideoDecoder* CreateVideoDecoder(webrtc::VideoCodecType type) override;

    void DestroyVideoDecoder(webrtc::VideoDecoder* decoder) override;

private:
    std::vector<webrtc::VideoCodecType> supported_codec_types_;
    std::unique_ptr<ics::base::VideoDecoderInterface> external_decoder_;
};

#endif  // ICS_BASE_CUSTOMIZEDVIDEODECODERFACTORY_H_