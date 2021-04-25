// Copyright (C) <2020> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_WIN_VIDEORENDERERD3D11_H
#define OWT_BASE_WIN_VIDEORENDERERD3D11_H

#include <windows.h>
#include <atlbase.h>
#include <combaseapi.h>

#include <d3d11.h>
#include <d3d11_1.h>
#include <d3d11_2.h>
#include <dcomp.h>
#include <dxgi1_2.h>

#ifdef ALLOW_TEARING
#include <dxgi1_4.h>
#include <dxgi1_5.h>
#endif

#include <dxgiformat.h>
#include <mfidl.h>
#include <psapi.h>
#include <wrl.h>
#include <vector>

#include "talk/owt/sdk/base/win/vpedefs.h"
#include "webrtc/api/video/video_frame.h"
#include "webrtc/api/video/video_sink_interface.h"
#include "webrtc/rtc_base/thread.h"
#include "webrtc/rtc_base/synchronization/mutex.h"
#include "webrtc/system_wrappers/include/clock.h"

namespace owt {
namespace base {

class WebrtcVideoRendererD3D11Impl
    : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
 public:
  WebrtcVideoRendererD3D11Impl(HWND wnd);
  virtual void OnFrame(const webrtc::VideoFrame& frame) override;
  virtual ~WebrtcVideoRendererD3D11Impl() {}

 private:
  bool InitMPO(int width, int height);
  void RenderNativeHandleFrame(const webrtc::VideoFrame& video_frame);
  void RenderNV12DXGIMPO(int width, int height);
  bool CreateVideoProcessor(int width, int height, bool reset);
  void RenderD3D11Texture(int width, int height);
  void RenderI420Frame_DX11(const webrtc::VideoFrame& video_frame);
  bool InitD3D11(int width, int height);
  bool InitSwapChain(int widht, int height, bool reset);
  bool CreateStagingTexture(int width, int height);
  bool GetWindowSizeForSwapChain(int& width, int& height);
  bool SupportSuperResolution();

  // Render window objects
  HWND wnd_ = nullptr;
  int window_width_ = 0;
  int window_height_ = 0;

  // D3D11 objects
  ID3D10Multithread* p_mt = nullptr;
  ID3D11Device* d3d11_device_ = nullptr;
  ID3D11Device2* d3d11_device2_ = nullptr;
  ID3D11VideoDevice* d3d11_video_device_ = nullptr;
  ID3D11DeviceContext* d3d11_device_context_ = nullptr;
  ID3D11VideoContext* d3d11_video_context_ = nullptr;
  ID3D11DeviceContext1* dx11_device_context1_ = nullptr;

  CComPtr<IDXGIFactory2> dxgi_factory_;
  CComPtr<IDXGISwapChain1> swap_chain_for_hwnd_;

  // MPO objects
  CComPtr<IDXGIDevice> dxgi_device2_;
  CComPtr<IDCompositionDevice2> comp_device2_;
  CComPtr<IDCompositionTarget> comp_target_;
  CComPtr<IDCompositionVisual2> root_visual_;
  CComPtr<IDCompositionVisual2> visual_preview_;

  // VideoProcessor objects
  CComPtr<ID3D11VideoProcessor> video_processor_;
  CComPtr<ID3D11VideoProcessorEnumerator> video_processor_enum_;

  // Handle of texture holding decoded image.
  webrtc::Mutex d3d11_texture_lock_;
  ID3D11Texture2D* d3d11_texture_ = nullptr;
  ID3D11Texture2D* d3d11_staging_texture_ = nullptr;
  D3D11_TEXTURE2D_DESC d3d11_texture_desc_;
  // Local view is using normal d3d11 swapchain.
  bool d3d11_raw_inited_ = false;
  // Remote view is using MPO swapchain.
  bool d3d11_mpo_inited_ = false;
  // Support super resolution
  bool sr_enabled_ = false;
  webrtc::Clock* clock_;
};

}  // namespace base
}  // namespace owt
#endif  // OWT_BASE_WIN_VIDEORENDERERD3D11_H