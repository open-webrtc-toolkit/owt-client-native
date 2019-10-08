// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "talk/owt/sdk/base/customizedvideodecoderfactory.h"
#include "owt/base/globalconfiguration.h"
CustomizedVideoDecoderFactory::CustomizedVideoDecoderFactory(std::unique_ptr<owt::base::VideoDecoderInterface> external_decoder)
  : external_decoder_(std::move(external_decoder)) {
  supported_codec_types_.clear();
  supported_codec_types_.push_back(webrtc::kVideoCodecH264);
  supported_codec_types_.push_back(webrtc::kVideoCodecH265);
  supported_codec_types_.push_back(webrtc::kVideoCodecVP8);
  supported_codec_types_.push_back(webrtc::kVideoCodecVP9);
}
CustomizedVideoDecoderFactory::~CustomizedVideoDecoderFactory() {}
webrtc::VideoDecoder* CustomizedVideoDecoderFactory::CreateVideoDecoder(webrtc::VideoCodecType type) {
  if (supported_codec_types_.empty()) {
    return nullptr;
  }
  for (std::vector<webrtc::VideoCodecType>::const_iterator it =
    supported_codec_types_.begin(); it != supported_codec_types_.end();
    ++it) {
      if (*it == type) {
        return new owt::base::CustomizedVideoDecoderProxy(type, external_decoder_->Copy());
      }
  }
  return nullptr;
}
void CustomizedVideoDecoderFactory::DestroyVideoDecoder(
    webrtc::VideoDecoder* decoder) {
    delete decoder;
}
