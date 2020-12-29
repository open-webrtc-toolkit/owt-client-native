// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/base/win/videorendererd3d11.h"
#include <array>
#include "rtc_base/logging.h"
#include "talk/owt/sdk/base/nativehandlebuffer.h"
#include "talk/owt/sdk/base/win/d3dnativeframe.h"
#include "talk/owt/sdk/include/cpp/owt/base/videorendererinterface.h"
#include "webrtc/common_video/libyuv/include/webrtc_libyuv.h"

// No. of vertices to draw a 3D QUAD
static const UINT num_vertices = 6;

using namespace rtc;
namespace owt {
namespace base {

WebrtcVideoRendererD3D11Impl::WebrtcVideoRendererD3D11Impl(HWND wnd)
    : wnd_(wnd), width_(0), height_(0) {
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

    uint16_t width = video_frame.video_frame_buffer()->width();
    uint16_t height = video_frame.video_frame_buffer()->height();

    if (width == 0 || height == 0)
      return;

    width_ = width;
    height_ = height;

    if (render_device != d3d11_device_) {
      d3d11_device_ = render_device;
      need_swapchain_recreate_ = true;
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
  }
  return;
}

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

  // Set viewport
  D3D11_VIEWPORT viewport = {0};
  viewport.TopLeftX = x_offset_;
  viewport.TopLeftY = y_offset_;
  viewport.Width = width_;
  viewport.Height = height_;
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
