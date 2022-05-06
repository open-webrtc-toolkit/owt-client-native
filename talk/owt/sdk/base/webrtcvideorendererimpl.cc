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
#include "talk/owt/sdk/base/nativehandlebuffer.h"
#include "talk/owt/sdk/base/webrtcvideorendererimpl.h"
#if defined(WEBRTC_WIN)
#include "talk/owt/sdk/base/win/d3dnativeframe.h"
#include "talk/owt/sdk/include/cpp/owt/base/videorendererinterface.h"
#endif
#include "webrtc/common_video/libyuv/include/webrtc_libyuv.h"

namespace owt {
namespace base {
void WebrtcVideoRendererImpl::OnFrame(const webrtc::VideoFrame& frame) {
  if (frame.video_frame_buffer()->type() ==
          webrtc::VideoFrameBuffer::Type::kNative) {
#if defined(WEBRTC_WIN)
    // This is D3D11Handle passed. Pass that directly to the attached renderer.
    D3D11VAHandle* native_handle = reinterpret_cast<D3D11VAHandle*>(
        reinterpret_cast<owt::base::NativeHandleBuffer*>(
            frame.video_frame_buffer().get())
            ->native_handle());

    if (native_handle == nullptr)
      return;

    ID3D11Device* render_device = native_handle->d3d11_device;
    ID3D11VideoDevice* render_video_device = native_handle->d3d11_video_device;
    ID3D11Texture2D* texture = native_handle->texture;
    ID3D11VideoContext* render_context = native_handle->context;
    int index = native_handle->array_index;

    uint16_t width = frame.video_frame_buffer()->width();
    uint16_t height = frame.video_frame_buffer()->height();

    if (width == 0 || height == 0)
      return;

    if (render_device == nullptr || render_video_device == nullptr || texture == nullptr)
      return;

    D3D11VAHandle* render_ptr = new D3D11VAHandle();
    render_ptr->texture = texture;
    render_ptr->array_index = index;
    render_ptr->d3d11_device = render_device;
    render_ptr->d3d11_video_device = render_video_device;
    render_ptr->context = render_context;
    render_ptr->side_data_size = native_handle->side_data_size;
    if (native_handle->side_data_size > 0)
      memcpy(&render_ptr->side_data[0], &native_handle->side_data[0],
             native_handle->side_data_size);
    if (native_handle->cursor_data_size > 0)
      memcpy(&render_ptr->cursor_data[0], &native_handle->cursor_data[0],
             native_handle->cursor_data_size);
    Resolution resolution(frame.width(), frame.height());
    std::unique_ptr<VideoBuffer> video_buffer(new VideoBuffer{
        (uint8_t*)render_ptr, resolution, VideoBufferType::kD3D11});

    renderer_.RenderFrame(std::move(video_buffer));
#else
    return;
#endif
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
