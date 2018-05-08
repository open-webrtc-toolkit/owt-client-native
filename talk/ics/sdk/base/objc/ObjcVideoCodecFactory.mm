/*
 * Intel License
 */

#include "talk/ics/sdk/base/objc/ObjcVideoCodecFactory.h"
#include "webrtc/sdk/objc/Framework/Native/api/video_decoder_factory.h"
#include "webrtc/sdk/objc/Framework/Native/api/video_encoder_factory.h"
#include "webrtc/sdk/objc/Framework/Native/src/objc_video_decoder_factory.h"
#include "webrtc/sdk/objc/Framework/Native/src/objc_video_encoder_factory.h"

#import "talk/ics/sdk/base/objc/ICSDefaultVideoDecoderFactory.h"
#import "talk/ics/sdk/base/objc/ICSDefaultVideoEncoderFactory.h"
#import "WebRTC/RTCVideoCodecH264.h"

namespace ics {
namespace base {
std::unique_ptr<webrtc::VideoEncoderFactory>
ObjcVideoCodecFactory::CreateObjcVideoEncoderFactory() {
  return webrtc::ObjCToNativeVideoEncoderFactory(
      [[ICSDefaultVideoEncoderFactory alloc] init]);
}
std::unique_ptr<webrtc::VideoDecoderFactory>
ObjcVideoCodecFactory::CreateObjcVideoDecoderFactory() {
  return webrtc::ObjCToNativeVideoDecoderFactory(
      [[ICSDefaultVideoDecoderFactory alloc] init]);
}
}
}
