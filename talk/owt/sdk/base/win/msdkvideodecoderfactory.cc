// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/base/win/msdkvideodecoderfactory.h"
#include "talk/owt/sdk/base/win/msdkvideodecoder.h"

namespace owt {
namespace base {

MSDKVideoDecoderFactory::MSDKVideoDecoderFactory() {
  supported_codec_types_.clear();
  
  supported_codec_types_.push_back(webrtc::kVideoCodecVP8);

  bool is_h264_hw_supported = true;
  if (is_h264_hw_supported) {
    supported_codec_types_.push_back(webrtc::kVideoCodecH264);
  }
#ifndef DISABLE_H265
// TODO: Add logic to detect plugin by MSDK.
  bool is_h265_hw_supported = true;
  if (is_h265_hw_supported) {
    supported_codec_types_.push_back(webrtc::kVideoCodecH265);
  }
#endif
}

MSDKVideoDecoderFactory::~MSDKVideoDecoderFactory() {
}

webrtc::VideoDecoder* MSDKVideoDecoderFactory::CreateVideoDecoder(webrtc::VideoCodecType type) {
  if (supported_codec_types_.empty()) {
    return nullptr;
  }
  for (std::vector<webrtc::VideoCodecType>::const_iterator it =
    supported_codec_types_.begin(); it != supported_codec_types_.end();
    ++it) {
    if (*it == type && type == webrtc::kVideoCodecVP8) {
      return new owt::base::MSDKVideoDecoder(type);
    } else if (*it == type && type == webrtc::kVideoCodecH264) {
      return new owt::base::MSDKVideoDecoder(type);
#ifndef DISABLE_H265
    } else if (*it == type && type == webrtc::kVideoCodecH265) {
      return new MSDKVideoDecoder(type);
#endif
    }
  }
  return nullptr;
}

void MSDKVideoDecoderFactory::DestroyVideoDecoder(webrtc::VideoDecoder* decoder) {
  delete decoder;
}
}  // namespace base
}  // namespace owt
