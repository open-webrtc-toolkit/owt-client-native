/*************************************************************************
Copyright (C) 2018 Intel Corporation. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "talk/oms/sdk/base/win/msdkvideodecoderfactory.h"
#include "talk/oms/sdk/base/win/msdkvideodecoder.h"

namespace oms {
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
      return new oms::base::MSDKVideoDecoder(type);
    } else if (*it == type && type == webrtc::kVideoCodecH264) {
      return new oms::base::MSDKVideoDecoder(type);
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
}  // namespace oms
