/*
 * Intel License
 */
#ifndef WOOGEEN_BASE_WIN_VIDEORENDERER_WIN_H
#define WOOGEEN_BASE_WIN_VIDEORENDERER_WIN_H
#include <d3d9.h>
#include <d3d9types.h>
#include <atlbase.h>
#include <codecapi.h>
#include <combaseapi.h>
#include <dxva2api.h>
#include <Windows.h>
#include "webrtc/api/mediastreaminterface.h"
#include "webrtc/api/video/video_frame.h"
#include "webrtc/rtc_base/scoped_ref_ptr.h"
#include "webrtc/media/base/videosinkinterface.h"

namespace woogeen {
namespace base {

class WebrtcVideoRendererD3D9Impl
    : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
 public:
  WebrtcVideoRendererD3D9Impl(HWND wnd)
      : wnd_(wnd),
        first_frame_(true),
        inited_for_raw_(false),
        width_(0),
        height_(0) {}
  virtual void OnFrame(const webrtc::VideoFrame& frame) override;
  virtual ~WebrtcVideoRendererD3D9Impl() { Destroy(); }

 private:
  void Destroy();
  void Resize(size_t width, size_t height);
  HWND wnd_;
  bool first_frame_;
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
#endif  // WOOGEEN_BASE_WIN_VIDEORENDERER_WIN_H
