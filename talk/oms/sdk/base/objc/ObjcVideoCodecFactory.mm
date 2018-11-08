/*
 * Intel License
 */
#include "talk/oms/sdk/base/objc/ObjcVideoCodecFactory.h"
#include "webrtc/sdk/objc/Framework/Native/api/video_decoder_factory.h"
#include "webrtc/sdk/objc/Framework/Native/api/video_encoder_factory.h"
#include "webrtc/sdk/objc/Framework/Native/src/objc_video_decoder_factory.h"
#include "webrtc/sdk/objc/Framework/Native/src/objc_video_encoder_factory.h"
#import "talk/oms/sdk/base/objc/OMSDefaultVideoDecoderFactory.h"
#import "talk/oms/sdk/base/objc/OMSDefaultVideoEncoderFactory.h"
#import "WebRTC/RTCVideoCodecH264.h"
namespace oms {
namespace base {
std::unique_ptr<webrtc::VideoEncoderFactory>
ObjcVideoCodecFactory::CreateObjcVideoEncoderFactory() {
  return webrtc::ObjCToNativeVideoEncoderFactory(
      [[OMSDefaultVideoEncoderFactory alloc] init]);
}
std::unique_ptr<webrtc::VideoDecoderFactory>
ObjcVideoCodecFactory::CreateObjcVideoDecoderFactory() {
  return webrtc::ObjCToNativeVideoDecoderFactory(
      [[OMSDefaultVideoDecoderFactory alloc] init]);
}
}
}
