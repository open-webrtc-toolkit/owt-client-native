// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_OBJC_OBJCVIDEOCODECFACTORY_H_
#define OWT_BASE_OBJC_OBJCVIDEOCODECFACTORY_H_
#include <memory>
#include "webrtc/api/video_codecs/video_encoder_factory.h"
#include "webrtc/api/video_codecs/video_decoder_factory.h"
namespace owt {
namespace base {
class ObjcVideoCodecFactory {
 public:
  static std::unique_ptr<webrtc::VideoEncoderFactory>
  CreateObjcVideoEncoderFactory();
  static std::unique_ptr<webrtc::VideoDecoderFactory>
  CreateObjcVideoDecoderFactory();
};
}  // namespace base
}  // namespace owt
#endif
