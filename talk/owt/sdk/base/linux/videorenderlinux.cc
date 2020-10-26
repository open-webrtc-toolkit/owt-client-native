// Copyright (C) <2020> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "talk/owt/sdk/base/linux/videorenderlinux.h"
#include "talk/owt/sdk/base/linux/xwindownativeframe.h"
#include "talk/owt/sdk/base/nativehandlebuffer.h"
#include "talk/owt/sdk/base/webrtcvideorendererimpl.h"
#include "webrtc/common_video/libyuv/include/webrtc_libyuv.h"

namespace owt {
namespace base {

#if defined(WEBRTC_LINUX)
void WebrtcVideoRendererVaImpl::OnFrame(const webrtc::VideoFrame& frame) {
  if (frame.video_frame_buffer()->type() !=
      webrtc::VideoFrameBuffer::Type::kNative)
    return;
  NativeHandleBuffer* buffer =
      reinterpret_cast<NativeHandleBuffer*>(frame.video_frame_buffer().get());
  if (buffer == nullptr)
    return;
  NativeXWindowSurfaceHandle* native_handle =
      reinterpret_cast<NativeXWindowSurfaceHandle*>(buffer->native_handle());

  std::unique_ptr<VaSurface> va_surface(
      new VaSurface{native_handle->display_, native_handle->surface_,
                    native_handle->width_, native_handle->height_,
		    native_handle->frameno, native_handle->bufferid,
		    native_handle->data, native_handle->pfnReturnBuffer});
  renderer_.RenderFrame(std::move(va_surface));
}
#endif

}  // namespace base
}  // namespace owt
