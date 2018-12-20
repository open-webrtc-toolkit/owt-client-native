// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OMS_BASE_CUSTOMIZEDVIDEODECODERFACTORY_H_
#define OMS_BASE_CUSTOMIZEDVIDEODECODERFACTORY_H_
#include "webrtc/media/engine/webrtcvideodecoderfactory.h"
#include "talk/oms/sdk/base/customizedvideodecoderproxy.h"
#include "talk/oms/sdk/include/cpp/oms/base/videodecoderinterface.h"
#pragma once
// Declaration of customized based decoder factory.
class CustomizedVideoDecoderFactory
    : public cricket::WebRtcVideoDecoderFactory {
public:
    CustomizedVideoDecoderFactory(std::unique_ptr<oms::base::VideoDecoderInterface> external_decoder);
    virtual ~CustomizedVideoDecoderFactory();
    // WebRtcVideoDecoderFactory implementation.
    webrtc::VideoDecoder* CreateVideoDecoder(webrtc::VideoCodecType type) override;
    void DestroyVideoDecoder(webrtc::VideoDecoder* decoder) override;
private:
    std::vector<webrtc::VideoCodecType> supported_codec_types_;
    std::unique_ptr<oms::base::VideoDecoderInterface> external_decoder_;
};
#endif  // OMS_BASE_CUSTOMIZEDVIDEODECODERFACTORY_H_