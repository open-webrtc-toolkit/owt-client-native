/*
 * Intel License
 */
#if defined(WEBRTC_WIN)
#include <atlbase.h>
#include <codecapi.h>
#include <combaseapi.h>
#include <d3d9.h>
#include <dxva2api.h>
#include <Windows.h>
#endif
#include "talk/ics/sdk/base/webrtcvideorendererimpl.h"
#if defined(WEBRTC_WIN)
#include "talk/ics/sdk/base/win/d3dnativeframe.h"
#endif
#include "webrtc/common_video/libyuv/include/webrtc_libyuv.h"
#include "webrtc/media/base/videocommon.h"

namespace ics {
namespace base {

void WebrtcVideoRendererARGBImpl::OnFrame(const webrtc::VideoFrame& frame) {
  if (frame.video_frame_buffer()->type() ==
    webrtc::VideoFrameBuffer::Type::kNative) {
    return;
  }
  Resolution resolution(frame.width(),frame.height());
  uint8_t* buffer = new uint8_t[resolution.width * resolution.height * 4];
  webrtc::ConvertFromI420(frame, webrtc::VideoType::kARGB, 0, static_cast<uint8_t*>(buffer));
  std::unique_ptr<ARGBBuffer> argb_buffer(new ARGBBuffer{buffer, resolution});
  renderer_.RenderFrame(std::move(argb_buffer));
}

}
}
