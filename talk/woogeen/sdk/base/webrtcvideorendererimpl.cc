/*
 * Intel License
 */

#include "talk/media/base/videocommon.h"
#include "talk/woogeen/sdk/base/webrtcvideorendererimpl.h"

namespace woogeen {
namespace base {
void WebrtcVideoRendererRGBImpl::RenderFrame(
    const cricket::VideoFrame* video_frame) {
  const cricket::VideoFrame* frame = video_frame->GetCopyWithRotationApplied();
  Resolution resolution(static_cast<int>(frame->GetWidth()),
                        static_cast<int>(frame->GetHeight()));
  uint8_t* buffer = new uint8_t[resolution.width * resolution.height * 4];
  frame->ConvertToRgbBuffer(cricket::FOURCC_ARGB, buffer,
                            resolution.width * resolution.height * 4,
                            resolution.width * 4);
  std::unique_ptr<RGBBuffer> rgb_buffer(new RGBBuffer{buffer, resolution});
  renderer_.RenderFrame(std::move(rgb_buffer));
}
}
}
