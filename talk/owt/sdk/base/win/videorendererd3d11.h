// Copyright (C) <2020> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_WIN_VIDEORENDERERD3D11_H
#define OWT_BASE_WIN_VIDEORENDERERD3D11_H
#include <windows.h>
#include <atlbase.h>
#include <combaseapi.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include "webrtc/api/video/video_frame.h"
#include "webrtc/api/video/video_sink_interface.h"
namespace owt {
namespace base {
class WebrtcVideoRendererD3D11Impl
    : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
 public:
  WebrtcVideoRendererD3D11Impl(HWND wnd);
  virtual void OnFrame(const webrtc::VideoFrame& frame) override;
  virtual ~WebrtcVideoRendererD3D11Impl() { Destroy(); }

 private:
  void Destroy();
  void Resize(size_t width, size_t height);
  bool CreateOrUpdateContext(ID3D11Device* device,
      ID3D11VideoDevice* video_device, ID3D11VideoContext* context,
      uint16_t width, uint16_t height);
  void FillSwapChainDesc(DXGI_SWAP_CHAIN_DESC1& scd);
  bool CreateVideoProcessor(uint16_t width, uint16_t height);
  bool RenderFrame(ID3D11Texture2D* texture);

  HWND wnd_ = nullptr;
  uint16_t width_ = 0;
  uint16_t height_ = 0;

  // Owner of the d3d11 device/context is decoder.
  ID3D11Device* d3d11_device_ = nullptr;
  ID3D11VideoDevice* d3d11_video_device_ = nullptr;
  ID3D11VideoContext* d3d11_video_context_ = nullptr;

  CComPtr<IDXGIFactory2> dxgi_factory_;
  CComPtr<ID3D11VideoProcessorEnumerator> video_processors_enum_;
  CComPtr<ID3D11VideoProcessor> video_processor_;
  CComPtr<ID3D11VideoProcessorInputView> input_view_;
  CComPtr<ID3D11VideoProcessorOutputView> output_view_;
  CComPtr<IDXGISwapChain1> swap_chain_for_hwnd_;
};
}  // namespace base
}  // namespace owt
#endif  // OWT_BASE_WIN_VIDEORENDERERD3D11_H