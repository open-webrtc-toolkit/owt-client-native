// Copyright (C) <2020> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_WIN_VIDEORENDERERD3D11_H
#define OWT_BASE_WIN_VIDEORENDERERD3D11_H

#include <windows.h>
#include <atlbase.h>
#include <combaseapi.h>
#include <d3d9.h>
#include <d3d9types.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <dxgi1_2.h>
#include <dxgi1_4.h>
#include <dxgi1_5.h>
#include <dxgiformat.h>
#include <psapi.h>
#include "talk/owt/sdk/base/win/shader.h"
#include <vector>
#include "webrtc/api/video/video_frame.h"
#include "webrtc/api/video/video_sink_interface.h"

// Holds texture coordinate of the vertex
typedef struct _TEXCOORD {
  float u;
  float v;
} TEXCOORD;

// Holds 3D position of the vertex
typedef struct _POSITION {
  float x;
  float y;
  float z;
} POSITION;

// Holds information of each vertex
typedef struct _VERTEX {
  POSITION pos;
  TEXCOORD tex_coord;
} VERTEX;

// Shader resource view for each input texture
typedef struct _TextureView {
  ID3D11ShaderResourceView* luma_view = nullptr;
  ID3D11ShaderResourceView* chroma_view = nullptr;
} TextureView;

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
  void FillSwapChainDesc(DXGI_SWAP_CHAIN_DESC1& scd);
  void ResetTextureViews();
#if 0
  HRESULT CreateD3D11Device();
  void WriteNV12ToTexture();
#endif
  HRESULT CreateRenderPipeline();
  HRESULT ResizeRenderPipeline();
  void ResizeD3D9RenderPipeline(size_t width, size_t height);
  HRESULT CreateTextureView(ID3D11Texture2D* texture, int array_slice);
  void RenderToBackbuffer(int array_slice);
  void InitializeSRVDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc,
                         ID3D11Texture2D* texture,
                         D3D11_SRV_DIMENSION view_dimension,
                         DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN,
                         UINT most_detailed_mip = 0,
                         UINT mip_levels = -1,
                         UINT first_array_slice = 0,
                         UINT array_size = -1);

  HWND wnd_ = nullptr;
  uint16_t x_offset_ = 0;
  uint16_t y_offset_ = 0;
  uint16_t width_ = 0;
  uint16_t height_ = 0;
  int window_width = 0;
  int window_height = 0;
  bool need_swapchain_recreate_ = true;

  // Owner of the d3d11 device/context is decoder.
  ID3D11Device* d3d11_device_ = nullptr;
  ID3D11VideoDevice* d3d11_video_device_ = nullptr;
  ID3D11DeviceContext* d3d11_device_context_ = nullptr;
  ID3D11VideoContext* d3d11_video_context_ = nullptr;
  TextureView* texture_view_ = nullptr;
  ID3D11DeviceContext1* dx11_device_context1_;

  CComPtr<IDXGIFactory2> dxgi_factory_;
  CComPtr<IDXGISwapChain1> swap_chain_for_hwnd_;

  ID3D11RenderTargetView* render_target_view_ = nullptr;
  ID3D11SamplerState* sampler_linear_ = nullptr;
  ID3D11BlendState* blend_state_ = nullptr;
  ID3D11VertexShader* vertex_shader_ = nullptr;
  ID3D11PixelShader* pixel_shader_ = nullptr;
  ID3D11InputLayout* input_layout_ = nullptr;
  ID3D11Buffer* vertex_buffer_ = nullptr;

  // Using D3D9 for rendering SW frames.
  bool d3d9_inited_for_raw_ = false;
  rtc::scoped_refptr<IDirect3D9> m_d3d_;
  rtc::scoped_refptr<IDirect3DDevice9> m_d3d_device_;
  rtc::scoped_refptr<IDirect3DTexture9> m_texture_;
  rtc::scoped_refptr<IDirect3DVertexBuffer9> m_vertex_buffer_;
  UINT views_count = 0;

};
}  // namespace base
}  // namespace owt
#endif  // OWT_BASE_WIN_VIDEORENDERERD3D11_H