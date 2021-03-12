// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/base/win/videorendererd3d11.h"
#include <cstdio>
#include <array>
#include "rtc_base/logging.h"
#include "talk/owt/sdk/base/nativehandlebuffer.h"
#include "talk/owt/sdk/base/win/d3dnativeframe.h"
#include "talk/owt/sdk/include/cpp/owt/base/videorendererinterface.h"
#include "webrtc/common_video/libyuv/include/webrtc_libyuv.h"

// No. of vertices to draw a 3D QUAD
static const UINT num_vertices = 6;
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_TEX1)
struct D3dCustomVertex {
  float x, y, z;
  float u, v;
};

static int frame_count = 0;
using namespace rtc;
namespace owt {
namespace base {

WebrtcVideoRendererD3D11Impl::WebrtcVideoRendererD3D11Impl(HWND wnd)
    : wnd_(wnd), width_(0), height_(0), clock_(Clock::GetRealTimeClock()) {
  CreateDXGIFactory(__uuidof(IDXGIFactory2), (void**)(&dxgi_factory_));
}

void WebrtcVideoRendererD3D11Impl::Destroy() {
  ResetTextureViews();
}

void WebrtcVideoRendererD3D11Impl::FillSwapChainDesc(
    DXGI_SWAP_CHAIN_DESC1& scd) {
  scd.Width = scd.Height = 0;  // automatic sizing.
  scd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
  scd.Stereo = false;
  scd.SampleDesc.Count = 1;  // no multi-sampling
  scd.SampleDesc.Quality = 0;
  scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  scd.BufferCount = 2;
  scd.Scaling = DXGI_SCALING_STRETCH;
  scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
  scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
}

void WebrtcVideoRendererD3D11Impl::OnFrame(
    const webrtc::VideoFrame& video_frame) {
  uint16_t width = video_frame.video_frame_buffer()->width();
  uint16_t height = video_frame.video_frame_buffer()->height();

  RTC_LOG(LS_ERROR) << "On frame." << width << "x" << height;
  if (width == 0 || height == 0)
    return;

  if (video_frame.video_frame_buffer()->type() ==
      webrtc::VideoFrameBuffer::Type::kNative) {
    D3D11ImageHandle* native_handle =
        reinterpret_cast<D3D11ImageHandle*>(
            reinterpret_cast<owt::base::NativeHandleBuffer*>(
                video_frame.video_frame_buffer().get())
                ->native_handle());

    if (native_handle == nullptr)
      return;

    ID3D11Device* render_device = native_handle->d3d11_device;

    if (!render_device)
      return;
    int array_slice = native_handle->texture_array_index;

    HRESULT hr = S_OK;
    ID3D11Texture2D* texture = native_handle->texture;

    // Validate window
    if (wnd_ && dxgi_factory_ && IsWindow(wnd_) && texture)
      hr = S_OK;
    else
      return;

    if (width_ != width || height_ != height) {
      width_ = width;
      height_ = height;
    }

    RECT rect;
    GetClientRect(wnd_, &rect);

    if (window_width != rect.right - rect.left ||
        window_height != rect.bottom - rect.top) {
      need_swapchain_recreate_ = true;
      window_width = rect.right - rect.left;
      window_height = rect.bottom - rect.top;
    }

    if (render_device != d3d11_device_) {
      need_swapchain_recreate_ = true;
      d3d11_device_ = render_device;
    }

    if (need_swapchain_recreate_) {
      if (swap_chain_for_hwnd_ == nullptr) {
        hr = CreateRenderPipeline();
      } else {
        hr = ResizeRenderPipeline();
      }
      need_swapchain_recreate_ = false;
    }

    if (SUCCEEDED(hr) && texture_view_ == nullptr) {
      hr = CreateTextureView(texture, array_slice);
    }

    if (SUCCEEDED(hr)) {
      RenderToBackbuffer(array_slice);
    }
  } else {  // I420 frame passed.
    if (!d3d9_inited_for_raw_) {
      m_d3d_ = Direct3DCreate9(D3D_SDK_VERSION);
      if (!m_d3d_)
        return;

      D3DPRESENT_PARAMETERS d3d_params = {};

      d3d_params.Windowed = true;
      d3d_params.SwapEffect = D3DSWAPEFFECT_DISCARD;

      // Check anti-alias support
      static D3DMULTISAMPLE_TYPE multisample_types[] = {
          D3DMULTISAMPLE_5_SAMPLES, D3DMULTISAMPLE_4_SAMPLES,
          D3DMULTISAMPLE_2_SAMPLES, D3DMULTISAMPLE_NONE};
      DWORD multisample_quality = 0;
      for (int i = 0; i < 4; i++) {
        HRESULT hr = m_d3d_->CheckDeviceMultiSampleType(
            D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_A8R8G8B8, true,
            multisample_types[i], &multisample_quality);
        if (SUCCEEDED(hr)) {
          d3d_params.MultiSampleType = multisample_types[i];
          d3d_params.MultiSampleQuality = multisample_quality - 1;
          break;
        }
      }

      IDirect3DDevice9* d3d_device;
      if (m_d3d_->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, wnd_,
                               D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3d_params,
                               &d3d_device) != D3D_OK) {
        Destroy();
        return;
      }
      m_d3d_device_ = d3d_device;
      d3d_device->Release();

      IDirect3DVertexBuffer9* vertex_buffer;
      const int kRectVertices = 4;
      if (m_d3d_device_->CreateVertexBuffer(
              kRectVertices * sizeof(D3dCustomVertex), 0, D3DFVF_CUSTOMVERTEX,
              D3DPOOL_MANAGED, &vertex_buffer, nullptr) != D3D_OK) {
        Destroy();
        return;
      }
      m_vertex_buffer_ = vertex_buffer;
      vertex_buffer->Release();

      m_d3d_device_->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
      m_d3d_device_->SetRenderState(D3DRS_LIGHTING, FALSE);
      m_d3d_device_->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
      ResizeD3D9RenderPipeline(video_frame.width(), video_frame.height());
      d3d9_inited_for_raw_ = true;
    } else {
      HRESULT hr = m_d3d_device_->TestCooperativeLevel();
      if (FAILED(hr)) {
        if (hr == D3DERR_DEVICELOST) {
          RTC_LOG(LS_WARNING) << "Device lost.";
        } else if (hr == D3DERR_DEVICENOTRESET) {
          Destroy();
          RTC_LOG(LS_WARNING) << "Device try to reinit.";
        } else {
          RTC_LOG(LS_WARNING) << "Device driver internal error.";
        }

        return;
      }

      if (static_cast<size_t>(video_frame.width()) != width_ ||
          static_cast<size_t>(video_frame.height()) != height_) {
        ResizeD3D9RenderPipeline(static_cast<size_t>(video_frame.width()),
               static_cast<size_t>(video_frame.height()));
      }
      D3DLOCKED_RECT lock_rect;
      if (m_texture_->LockRect(0, &lock_rect, nullptr, 0) != D3D_OK)
        return;

      ConvertFromI420(video_frame, webrtc::VideoType::kARGB, 0,
                      static_cast<uint8_t*>(lock_rect.pBits));
      m_texture_->UnlockRect(0);

      m_d3d_device_->BeginScene();
      m_d3d_device_->SetFVF(D3DFVF_CUSTOMVERTEX);
      m_d3d_device_->SetStreamSource(0, m_vertex_buffer_, 0,
                                     sizeof(D3dCustomVertex));
      m_d3d_device_->SetTexture(0, m_texture_);
      m_d3d_device_->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
      m_d3d_device_->EndScene();

      m_d3d_device_->Present(nullptr, nullptr, wnd_, nullptr);
    }
  }
  return;
}

void WebrtcVideoRendererD3D11Impl::ResizeD3D9RenderPipeline(size_t width, size_t height) {
  width_ = width;
  height_ = height;
  IDirect3DTexture9* texture;
  // Texture should alwasy be created with the size of frame.
  m_d3d_device_->CreateTexture(static_cast<UINT>(width_),
                               static_cast<UINT>(height_), 1, 0, D3DFMT_A8R8G8B8,
                               D3DPOOL_MANAGED, &texture, nullptr);
  m_texture_ = texture;
  texture->Release();

  // Vertices for the video frame to be rendered to.
  static const D3dCustomVertex rect[] = {
      {-1.0f, -1.0f, 0.0f, 0.0f, 1.0f},
      {-1.0f, 1.0f, 0.0f, 0.0f, 0.0f},
      {1.0f, -1.0f, 0.0f, 1.0f, 1.0f},
      {1.0f, 1.0f, 0.0f, 1.0f, 0.0f},
  };

  void* buf_data = nullptr;
  if (m_vertex_buffer_->Lock(0, 0, &buf_data, 0) != D3D_OK)
    return;

  CopyMemory(buf_data, &rect, sizeof(rect));
  m_vertex_buffer_->Unlock();
}

// TODO: Use D3D11 for rendering of I420 view as well.
#if 0
HRESULT WebrtcVideoRendererD3D11Impl::CreateD3D11Device() {
  HRESULT hr = S_OK;

  static D3D_FEATURE_LEVEL feature_levels[] = {
      D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1,
      D3D_FEATURE_LEVEL_10_1};
  D3D_FEATURE_LEVEL feature_levels_out;

    hr = D3D11CreateDevice(
      nullptr, D3D_DRIVER_TYPE_UNKNOWN, nullptr, 0, feature_levels,
      sizeof(feature_levels) / sizeof(feature_levels[0]), D3D11_SDK_VERSION,
      &d3d11_device_, &feature_levels_out, &d3d11_device_context_);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "Failed to create d3d11 device for decoder";
    return false;
  }

  // Create the surface to copy NV12 image to.
  D3D11_TEXTURE2D_DESC texture_desc;
  memset(&texture_desc, 0, sizeof(texture_desc));
  texture_desc.Format = DXGI_FORMAT_R8_UNORM;
  texture_desc.Width = width_;  // same as video width
  texture_desc.Height = height_; // same as video height
  texture_desc.ArraySize = 1;
  texture_desc.MipLevels = 1;
  texture_desc.Usage = D3D11_USAGE_DEFAULT; // We use UpdateSubresource to update the texture,
                                            // so not setting to D3D11_USAGE_DEFAULT;

  hr = d3d11_device_->CreateTexture2D(&texture_desc, nullptr,
                                      &texture_planes_[0]);
  
  texture_desc.Width = width_ / 2;
  texture_desc.Height = height_ / 2;
}
#endif

// Method to create the necessary 3D pipeline objects to render a textured 3D
// QUAD
HRESULT WebrtcVideoRendererD3D11Impl::CreateRenderPipeline() {
  DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {0};
  FillSwapChainDesc(swap_chain_desc);

  RTC_LOG(LS_INFO) << "Creating renderer of width [" << width_ << "] height ["
             << height_ << "]";

  // Create the swap chain for the window
  HRESULT hr = dxgi_factory_->CreateSwapChainForHwnd(
      d3d11_device_, wnd_, &swap_chain_desc, nullptr, nullptr,
      &swap_chain_for_hwnd_);

  if (SUCCEEDED(hr)) {
    ID3D11Texture2D* back_buffer = nullptr;

    // Get the back buffer from the swap chain
    hr = swap_chain_for_hwnd_->GetBuffer(
        0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&back_buffer));

    if (SUCCEEDED(hr)) {
      // Create the render target view
      hr = d3d11_device_->CreateRenderTargetView(back_buffer, nullptr,
                                                 &render_target_view_);
      back_buffer->Release();
      back_buffer = nullptr;
    }
    d3d11_device_->GetImmediateContext(&d3d11_device_context_);

    if (SUCCEEDED(hr)) {
      hr = d3d11_device_context_->QueryInterface(
          __uuidof(ID3D11DeviceContext1),
          reinterpret_cast<void**>(&dx11_device_context1_));
    }

    if (SUCCEEDED(hr)) {
      // Sampler
      D3D11_SAMPLER_DESC desc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
      hr = d3d11_device_->CreateSamplerState(&desc, &sampler_linear_);
      if (FAILED(hr)) {
        RTC_LOG(LS_ERROR) << "CreateSamplerState failed";
      }
    }

    UINT shader_blog_size = ARRAYSIZE(g_vertex_shader);

    if (SUCCEEDED(hr)) {
      // Vertex Shader
      hr = d3d11_device_->CreateVertexShader(g_vertex_shader, shader_blog_size,
                                             nullptr, &vertex_shader_);
      if (FAILED(hr)) {
        RTC_LOG(LS_ERROR) << "CreateVertexShader failed";
      }
    }

    if (SUCCEEDED(hr)) {
      // IA layout
      constexpr std::array<D3D11_INPUT_ELEMENT_DESC, 2> Layout = {{
          {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
           D3D11_INPUT_PER_VERTEX_DATA, 0},
          {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12,
           D3D11_INPUT_PER_VERTEX_DATA, 0},
      }};

      // Input assembler layout
      hr = d3d11_device_->CreateInputLayout(Layout.data(), Layout.size(),
                                            g_vertex_shader, shader_blog_size,
                                            &input_layout_);
      if (FAILED(hr)) {
        RTC_LOG(LS_ERROR) << "CreateInputLayout failed";
      }
    }

    if (SUCCEEDED(hr)) {
      // Vertices for render a 3D QUAD
      VERTEX vertices[num_vertices] = {
          {{-1.0f, -1.0f, 0}, {0.0f, 1.0f}}, {{-1.0f, 1.0f, 0}, {0.0f, 0.0f}},
          {{1.0f, -1.0f, 0}, {1.0f, 1.0f}},  {{1.0f, -1.0f, 0}, {1.0f, 1.0f}},
          {{-1.0f, 1.0f, 0}, {0.0f, 0.0f}},  {{1.0f, 1.0f, 0}, {1.0f, 0.0f}},
      };

      D3D11_BUFFER_DESC BufferDesc;
      RtlZeroMemory(&BufferDesc, sizeof(BufferDesc));
      BufferDesc.Usage = D3D11_USAGE_DEFAULT;
      BufferDesc.ByteWidth = sizeof(VERTEX) * num_vertices;
      BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
      BufferDesc.CPUAccessFlags = 0;
      D3D11_SUBRESOURCE_DATA InitData;
      RtlZeroMemory(&InitData, sizeof(InitData));
      InitData.pSysMem = vertices;

      // Vertex buffer
      hr = d3d11_device_->CreateBuffer(&BufferDesc, &InitData, &vertex_buffer_);
      if (FAILED(hr)) {
        RTC_LOG(LS_ERROR) << "CreateBuffer failed";
      }
    }

    if (SUCCEEDED(hr)) {
      // Pixel Shader
      shader_blog_size = ARRAYSIZE(g_pixel_shader);
      hr = d3d11_device_->CreatePixelShader(g_pixel_shader, shader_blog_size,
                                            nullptr, &pixel_shader_);
      if (FAILED(hr)) {
        RTC_LOG(LS_ERROR) << "CreatePixelShader failed";
      }
    }

    // Set new render target
    d3d11_device_context_->OMSetRenderTargets(1, &render_target_view_, nullptr);
    d3d11_device_context_->IASetInputLayout(input_layout_);
  }

  return (hr);
}

// Method to resize the objects when the window size changes
HRESULT WebrtcVideoRendererD3D11Impl::ResizeRenderPipeline() {
  HRESULT hr = S_FALSE;
  DXGI_SWAP_CHAIN_DESC desc = {0};

  RTC_LOG(LS_INFO) << "Resizing renderer to width [" << width_ << "] height ["
             << height_ << "]";

  hr = swap_chain_for_hwnd_->GetDesc(&desc);
  if (SUCCEEDED(hr)) {
    RTC_LOG(LS_INFO) << "Previous width [" << desc.BufferDesc.Width << "] height ["
               << desc.BufferDesc.Height << "]";
  }

  RECT rect;
  GetClientRect(wnd_, &rect);
  RTC_LOG(LS_INFO) << "Window width [" << (rect.right - rect.left) << "] height ["
             << (rect.bottom - rect.top) << "]";

  // Release the current render target view
  render_target_view_->Release();

  hr = swap_chain_for_hwnd_->ResizeBuffers(
      desc.BufferCount, (rect.right - rect.left), (rect.bottom - rect.top),
      desc.BufferDesc.Format, desc.Flags);

  if (SUCCEEDED(hr)) {
    ID3D11Texture2D* back_buffer = nullptr;
    HRESULT hr = swap_chain_for_hwnd_->GetBuffer(
        0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&back_buffer));

    // Create the render target view for the new back buffer
    hr = d3d11_device_->CreateRenderTargetView(back_buffer, nullptr, &render_target_view_);
    back_buffer->Release();
    back_buffer = nullptr;

    if (FAILED(hr)) {
      RTC_LOG(LS_ERROR) << "ResizeBuffers failed " << hr;
    }
  }

  return hr;
}

// Method to create the shader resource view for the incoming decoded video
// frame. We will bind the decoded video frame as a texture to the pixel shader
// stage in the 3D pipeline
HRESULT WebrtcVideoRendererD3D11Impl::CreateTextureView(
    ID3D11Texture2D* texture, int array_slice) {
  HRESULT hr = S_FALSE;

  texture_view_ = new TextureView;

  D3D11_SHADER_RESOURCE_VIEW_DESC luminance_plane_desc = {0};
  InitializeSRVDesc(luminance_plane_desc, texture,
                    D3D11_SRV_DIMENSION_TEXTURE2DARRAY, DXGI_FORMAT_R8_UNORM, 0,
                    1, array_slice, 1);

  hr = d3d11_device_->CreateShaderResourceView(
      texture, &luminance_plane_desc,
      &(texture_view_->luma_view));

  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "CreateShaderResourceView for luminance texture failed" << hr;
  } else {
    D3D11_SHADER_RESOURCE_VIEW_DESC chrominance_plane_desc = {0};

    InitializeSRVDesc(chrominance_plane_desc, texture,
                      D3D11_SRV_DIMENSION_TEXTURE2DARRAY,
                      DXGI_FORMAT_R8G8_UNORM, 0, 1, array_slice, 1);

    hr = d3d11_device_->CreateShaderResourceView(
        texture, &chrominance_plane_desc,
        &(texture_view_->chroma_view));

    if (FAILED(hr)) {
      RTC_LOG(LS_ERROR) << "CreateShaderResourceView for chrominance texture failed"
                 << hr;
    }
  }

  if (FAILED(hr)) {
    if (texture_view_->luma_view) {
      texture_view_->luma_view->Release();
      texture_view_->luma_view = nullptr;
    }

    if (texture_view_->chroma_view) {
      texture_view_->chroma_view->Release();
      texture_view_->chroma_view = nullptr;
    }

    delete texture_view_;
    texture_view_ = nullptr;
  }

  return hr;
}

// MSDK decode will always output with array_slice = 0, so we're not using
// a texture view array for this.
// TODO(jianlin): Create texture view array per slice if decoder(for example,
// dxva decoder) passes texture with array_slice > 0.
void WebrtcVideoRendererD3D11Impl::RenderToBackbuffer(int array_slice) {
  // Clear render target (RGBA value)
  FLOAT clear_color[4] = {0.0f, 0.0f, 0.0f, 0.0f};
  d3d11_device_context_->ClearRenderTargetView(render_target_view_, clear_color);

  RECT rect;
  GetClientRect(wnd_, &rect);

  // Set viewport
  D3D11_VIEWPORT viewport = {0};
  viewport.TopLeftX = 0;
  viewport.TopLeftY = 0;
  viewport.Width = rect.right - rect.left;
  viewport.Height = rect.bottom - rect.top;
  d3d11_device_context_->RSSetViewports(1, &viewport);

  // Bind the textures
  std::array<ID3D11ShaderResourceView*, 2> const shader_resource_views = {
      texture_view_->luma_view,
      texture_view_->chroma_view};

  // Bind the NV12 channels to the shader.
  d3d11_device_context_->PSSetShaderResources(0, shader_resource_views.size(),
                                              shader_resource_views.data());

  // Setup the 3D pipeline
  UINT stride = sizeof(VERTEX);
  UINT offset = 0;
  FLOAT blendFactor[4] = {0.f, 0.f, 0.f, 0.f};

  // Bind the blend state
  d3d11_device_context_->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
  // Bind the render target view
  d3d11_device_context_->OMSetRenderTargets(1, &render_target_view_, nullptr);
  // Bind the pass through vertex shader
  d3d11_device_context_->VSSetShader(vertex_shader_, nullptr, 0);
  // Bind the pixel shader
  d3d11_device_context_->PSSetShader(pixel_shader_, nullptr, 0);
  // Bind the sampler. We use a linear sampling
  d3d11_device_context_->PSSetSamplers(0, 1, &sampler_linear_);
  // Set the primitive topology to triangle list
  d3d11_device_context_->IASetPrimitiveTopology(
      D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  // Bind the vertex buffer to the pipeline
  d3d11_device_context_->IASetVertexBuffers(0, 1, &vertex_buffer_, &stride,
                                            &offset);

  // Draw textured quad onto render target
  d3d11_device_context_->Draw(num_vertices, 0);

  // Present the frame to the window
  DXGI_PRESENT_PARAMETERS parameters = {0};
  swap_chain_for_hwnd_->Present1(
      0, 0, &parameters);
  int64_t current_time = clock_->TimeInMilliseconds();
  frame_count++;
  FILE* f = fopen("frames.txt", "a");
  fprintf(f, "timestamp: %lld, frames drawn: %d\r\n", current_time, frame_count);
  fclose(f);
}

// Helper method to initialize a shader resource view
void WebrtcVideoRendererD3D11Impl::InitializeSRVDesc(
    D3D11_SHADER_RESOURCE_VIEW_DESC& desc,
                                    ID3D11Texture2D* texture,
                                    D3D11_SRV_DIMENSION view_dimension,
                                    DXGI_FORMAT format,
                                    UINT most_detailed_mip,
                                    UINT mip_levels,
                                    UINT first_array_slice,
                                    UINT array_size) {
  desc.ViewDimension = view_dimension;
  if (DXGI_FORMAT_UNKNOWN == format ||
      (-1 == mip_levels && D3D11_SRV_DIMENSION_TEXTURE2DMS != view_dimension &&
       D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY != view_dimension) ||
      (-1 == array_size &&
       (D3D11_SRV_DIMENSION_TEXTURE2DARRAY == view_dimension ||
        D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY == view_dimension ||
        D3D11_SRV_DIMENSION_TEXTURECUBEARRAY == view_dimension))) {
    D3D11_TEXTURE2D_DESC TexDesc;
    texture->GetDesc(&TexDesc);

    if (DXGI_FORMAT_UNKNOWN == format) {
      format = TexDesc.Format;
    }

    if (-1 == mip_levels) {
      mip_levels = TexDesc.MipLevels - most_detailed_mip;
    }

    if (-1 == array_size) {
      array_size = TexDesc.ArraySize - first_array_slice;
      if (D3D11_SRV_DIMENSION_TEXTURECUBEARRAY == view_dimension)
        array_size /= 6;
    }
  }
  desc.Format = format;
  switch (view_dimension) {
    case D3D11_SRV_DIMENSION_TEXTURE2D:
      desc.Texture2D.MostDetailedMip = most_detailed_mip;
      desc.Texture2D.MipLevels = mip_levels;
      break;
    case D3D11_SRV_DIMENSION_TEXTURE2DARRAY:
      desc.Texture2DArray.MostDetailedMip = most_detailed_mip;
      desc.Texture2DArray.MipLevels = mip_levels;
      desc.Texture2DArray.FirstArraySlice = first_array_slice;
      desc.Texture2DArray.ArraySize = array_size;
      break;
    case D3D11_SRV_DIMENSION_TEXTURE2DMS:
      break;
    case D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY:
      desc.Texture2DMSArray.FirstArraySlice = first_array_slice;
      desc.Texture2DMSArray.ArraySize = array_size;
      break;
    case D3D11_SRV_DIMENSION_TEXTURECUBE:
      desc.TextureCube.MostDetailedMip = most_detailed_mip;
      desc.TextureCube.MipLevels = mip_levels;
      break;
    case D3D11_SRV_DIMENSION_TEXTURECUBEARRAY:
      desc.TextureCubeArray.MostDetailedMip = most_detailed_mip;
      desc.TextureCubeArray.MipLevels = mip_levels;
      desc.TextureCubeArray.First2DArrayFace = first_array_slice;
      desc.TextureCubeArray.NumCubes = array_size;
      break;
    default:
      break;
  }
}
// Helper method to reset texture views
void WebrtcVideoRendererD3D11Impl::ResetTextureViews() {
  if (texture_view_) {
    delete texture_view_;
    texture_view_ = nullptr;
  }
}

}  // namespace base
}  // namespace owt
