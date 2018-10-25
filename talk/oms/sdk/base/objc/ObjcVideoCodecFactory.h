/*
 * Intel License
 */

#ifndef OMS_BASE_OBJC_OBJCVIDEOCODECFACTORY_H_
#define OMS_BASE_OBJC_OBJCVIDEOCODECFACTORY_H_

#include <memory>

#include "webrtc/api/video_codecs/video_encoder_factory.h"
#include "webrtc/api/video_codecs/video_decoder_factory.h"

namespace oms {
namespace base {

class ObjcVideoCodecFactory {
 public:
  static std::unique_ptr<webrtc::VideoEncoderFactory>
  CreateObjcVideoEncoderFactory();
  static std::unique_ptr<webrtc::VideoDecoderFactory>
  CreateObjcVideoDecoderFactory();
};
}  // namespace base
}  // namespace oms

#endif
