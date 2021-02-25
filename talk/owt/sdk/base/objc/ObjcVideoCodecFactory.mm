// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "talk/owt/sdk/base/objc/ObjcVideoCodecFactory.h"
#include "webrtc/sdk/objc/Framework/Native/api/video_decoder_factory.h"
#include "webrtc/sdk/objc/Framework/Native/api/video_encoder_factory.h"
#include "webrtc/sdk/objc/Framework/Native/src/objc_video_decoder_factory.h"
#include "webrtc/sdk/objc/Framework/Native/src/objc_video_encoder_factory.h"
#import "talk/owt/sdk/base/objc/OWTDefaultVideoDecoderFactory.h"
#import "talk/owt/sdk/base/objc/OWTDefaultVideoEncoderFactory.h"

namespace owt {
namespace base {
std::unique_ptr<webrtc::VideoEncoderFactory>
ObjcVideoCodecFactory::CreateObjcVideoEncoderFactory() {
  return webrtc::ObjCToNativeVideoEncoderFactory(
      [[OWTDefaultVideoEncoderFactory alloc] init]);
}
std::unique_ptr<webrtc::VideoDecoderFactory>
ObjcVideoCodecFactory::CreateObjcVideoDecoderFactory() {
  return webrtc::ObjCToNativeVideoDecoderFactory(
      [[OWTDefaultVideoDecoderFactory alloc] init]);
}
}
}
