/*
 * Intel License
 */

#include "talk/woogeen/sdk/base/webrtcvideorendererimpl.h"
#include "webrtc/common_video/libyuv/include/webrtc_libyuv.h"
#include "webrtc/media/base/videocommon.h"

namespace woogeen {
namespace base {

void WebrtcVideoRendererARGBImpl::OnFrame(const webrtc::VideoFrame& frame) {
  Resolution resolution(frame.width(),frame.height());
  uint8_t* buffer = new uint8_t[resolution.width * resolution.height * 4];
  webrtc::ConvertFromI420(frame, webrtc::VideoType::kARGB, 0, static_cast<uint8_t*>(buffer));
  std::unique_ptr<ARGBBuffer> argb_buffer(new ARGBBuffer{buffer, resolution});
  renderer_.RenderFrame(std::move(argb_buffer));
}
}
}
