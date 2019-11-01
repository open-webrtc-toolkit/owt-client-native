// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#if defined(WEBRTC_WIN)
#include <Windows.h>
#include <atlbase.h>
#include <codecapi.h>
#include <combaseapi.h>
#include <d3d9.h>
#include <dxva2api.h>
#endif
#include "talk/owt/sdk/base/webrtcvideorendererimpl.h"
#if defined(WEBRTC_WIN)
#include "talk/owt/sdk/base/win/d3dnativeframe.h"
#endif
#include "webrtc/common_video/libyuv/include/webrtc_libyuv.h"

namespace owt {
namespace base {
void WebrtcVideoRendererImpl::OnFrame(const webrtc::VideoFrame& frame) {
  if (frame.video_frame_buffer()->type() ==
          webrtc::VideoFrameBuffer::Type::kNative) {
    return;
  }
  VideoRendererType renderer_type = renderer_.Type();
  if (renderer_type != VideoRendererType::kI420 &&
      renderer_type != VideoRendererType::kARGB)
    return;
  Resolution resolution(frame.width(), frame.height());
  if (renderer_type == VideoRendererType::kARGB) {
    uint8_t* buffer = new uint8_t[resolution.width * resolution.height * 4];
    webrtc::ConvertFromI420(frame, webrtc::VideoType::kARGB, 0,
                            static_cast<uint8_t*>(buffer));
    std::unique_ptr<VideoBuffer> video_buffer(
        new VideoBuffer{buffer, resolution, VideoBufferType::kARGB});
    renderer_.RenderFrame(std::move(video_buffer));
  } else {
    uint8_t* buffer = new uint8_t[resolution.width * resolution.height * 3 / 2];
    webrtc::ConvertFromI420(frame, webrtc::VideoType::kI420, 0,
                            static_cast<uint8_t*>(buffer));
    std::unique_ptr<VideoBuffer> video_buffer(
        new VideoBuffer{buffer, resolution, VideoBufferType::kI420});
    renderer_.RenderFrame(std::move(video_buffer));
  }
}
}  // namespace base
}  // namespace owt
