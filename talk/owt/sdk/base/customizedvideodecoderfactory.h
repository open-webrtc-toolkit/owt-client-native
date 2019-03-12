// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_CUSTOMIZEDVIDEODECODERFACTORY_H_
#define OWT_BASE_CUSTOMIZEDVIDEODECODERFACTORY_H_
#include "webrtc/media/engine/webrtcvideodecoderfactory.h"
#include "talk/owt/sdk/base/customizedvideodecoderproxy.h"
#include "talk/owt/sdk/include/cpp/owt/base/videodecoderinterface.h"
#pragma once
// Declaration of customized based decoder factory.
class CustomizedVideoDecoderFactory
    : public cricket::WebRtcVideoDecoderFactory {
public:
    CustomizedVideoDecoderFactory(std::unique_ptr<owt::base::VideoDecoderInterface> external_decoder);
    virtual ~CustomizedVideoDecoderFactory();
    // WebRtcVideoDecoderFactory implementation.
    webrtc::VideoDecoder* CreateVideoDecoder(webrtc::VideoCodecType type) override;
    void DestroyVideoDecoder(webrtc::VideoDecoder* decoder) override;
private:
    std::vector<webrtc::VideoCodecType> supported_codec_types_;
    std::unique_ptr<owt::base::VideoDecoderInterface> external_decoder_;
};
#endif  // OWT_BASE_CUSTOMIZEDVIDEODECODERFACTORY_H_