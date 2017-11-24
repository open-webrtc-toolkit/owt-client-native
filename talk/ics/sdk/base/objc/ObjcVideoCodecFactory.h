/*
 * Intel License
 */

#ifndef WOOGEEN_BASE_OBJC_OBJCVIDEOCODECFACTORY_H_
#define WOOGEEN_BASE_OBJC_OBJCVIDEOCODECFACTORY_H_

#include <memory>

#include "webrtc/media/engine/webrtcvideodecoderfactory.h"
#include "webrtc/media/engine/webrtcvideoencoderfactory.h"

namespace ics {
namespace base {

class ObjcVideoCodecFactory {
 public:
  static std::unique_ptr<cricket::WebRtcVideoEncoderFactory>
  CreateObjcVideoEncoderFactory();
  static std::unique_ptr<cricket::WebRtcVideoDecoderFactory>
  CreateObjcVideoDecoderFactory();
};
}  // namespace base
}  // namespace ics

#endif
