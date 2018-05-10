/*
 * Intel License
 */

#include "talk/ics/sdk/base/objc/ObjcVideoCodecFactory.h"

#import "WebRTC/RTCVideoCodecH264.h"
#include "webrtc/sdk/objc/Framework/Native/src/objc_video_decoder_factory.h"
#include "webrtc/sdk/objc/Framework/Native/src/objc_video_encoder_factory.h"

namespace ics {
namespace base {
std::unique_ptr<cricket::WebRtcVideoEncoderFactory>
ObjcVideoCodecFactory::CreateObjcVideoEncoderFactory() {
  return std::unique_ptr<cricket::WebRtcVideoEncoderFactory>(
      new webrtc::ObjCVideoEncoderFactory(
          [[RTCVideoEncoderFactoryH264 alloc] init]));
}
std::unique_ptr<cricket::WebRtcVideoDecoderFactory>
ObjcVideoCodecFactory::CreateObjcVideoDecoderFactory() {
  return std::unique_ptr<cricket::WebRtcVideoDecoderFactory>(
      new webrtc::ObjCVideoDecoderFactory(
          [[RTCVideoDecoderFactoryH264 alloc] init]));
}
}
}
