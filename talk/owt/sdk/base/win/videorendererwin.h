// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_WIN_VIDEORENDERER_WIN_H
#define OWT_BASE_WIN_VIDEORENDERER_WIN_H
#include <d3d9.h>
#include <d3d9types.h>
#include <atlbase.h>
#include <codecapi.h>
#include <combaseapi.h>
#include <dxva2api.h>
#include <Windows.h>

#include "webrtc/api/video/video_sink_interface.h"
#include "webrtc/api/video/video_frame.h"
#include "webrtc/api/scoped_refptr.h"
namespace owt {
namespace base {
class WebrtcVideoRendererD3D9Impl
    : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
 public:
  WebrtcVideoRendererD3D9Impl(HWND wnd)
      : wnd_(wnd),
        inited_for_raw_(false),
        width_(0),
        height_(0) {}
  virtual void OnFrame(const webrtc::VideoFrame& frame) override;
  virtual ~WebrtcVideoRendererD3D9Impl() { Destroy(); }
 private:
  void Destroy();
  void Resize(size_t width, size_t height);
  HWND wnd_;
  bool inited_for_raw_;
  size_t width_;
  size_t height_;
  rtc::scoped_refptr<IDirect3D9> m_d3d_;
  rtc::scoped_refptr<IDirect3DDevice9> m_d3d_device_;
  rtc::scoped_refptr<IDirect3DTexture9> m_texture_;
  rtc::scoped_refptr<IDirect3DVertexBuffer9> m_vertex_buffer_;
};
}
}
#endif  // OWT_BASE_WIN_VIDEORENDERER_WIN_H
