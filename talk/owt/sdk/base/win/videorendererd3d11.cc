// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "rtc_base/logging.h"
#include "talk/owt/sdk/base/win/videorendererd3d11.h"
#include "talk/owt/sdk/base/nativehandlebuffer.h"
#include "talk/owt/sdk/base/win/d3d11device.h"
#include "talk/owt/sdk/base/win/d3dnativeframe.h"
#include "talk/owt/sdk/include/cpp/owt/base/videorendererinterface.h"
#include "webrtc/common_video/libyuv/include/webrtc_libyuv.h"

using namespace rtc;
namespace owt {
namespace base {

WebrtcVideoRendererD3D11Impl::WebrtcVideoRendererD3D11Impl(HWND wnd)
    : wnd_(wnd), width_(0), height_(0) {
  CreateDXGIFactory(__uuidof(IDXGIFactory2), (void**)(&dxgi_factory_));
}

void WebrtcVideoRendererD3D11Impl::Destroy() {}

void WebrtcVideoRendererD3D11Impl::Resize(size_t width, size_t height) {}

bool WebrtcVideoRendererD3D11Impl::CreateOrUpdateContext(
  ID3D11Device* device,
  ID3D11VideoDevice* video_device,
  ID3D11VideoContext* context, 
  uint16_t width, 
  uint16_t height) {
  if (!wnd_ || !dxgi_factory_.p || !IsWindow(wnd_))
    return false;

  if (device == nullptr || video_device == nullptr
      ||context == nullptr || width == 0 || height == 0)
    return false;

  if (device != d3d11_device_ || context != d3d11_video_context_) {
    // If device get changed, re-create the swap chain.
    d3d11_device_ = device;
    d3d11_video_device_ = video_device;
    d3d11_video_context_ = context;

    swap_chain_for_hwnd_.Release();

    DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {0};
    FillSwapChainDesc(swap_chain_desc);

    HRESULT hr = dxgi_factory_->CreateSwapChainForHwnd(
        d3d11_device_, wnd_, &swap_chain_desc, nullptr, nullptr,
        &swap_chain_for_hwnd_);

    if (FAILED(hr)) {
      return false;
    }

    D3D11_VIDEO_PROCESSOR_CONTENT_DESC content_desc;
    memset(&content_desc, 0, sizeof(content_desc));

    // Non-scaling
    content_desc.InputFrameFormat = D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE;
    content_desc.InputFrameRate.Numerator = 60;
    content_desc.InputFrameRate.Denominator = 1;
    content_desc.InputWidth = width;
    content_desc.InputHeight = height;
    content_desc.OutputWidth = width;
    content_desc.OutputHeight = height;
    content_desc.OutputFrameRate.Numerator = 60;
    content_desc.OutputFrameRate.Denominator = 1;
    content_desc.Usage = D3D11_VIDEO_USAGE_PLAYBACK_NORMAL;

    video_processors_enum_.Release();
    hr = d3d11_video_device_->CreateVideoProcessorEnumerator(
        &content_desc, &video_processors_enum_);
    if (FAILED(hr)) {
      return false;
    }
  }

  return true;
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

bool WebrtcVideoRendererD3D11Impl::CreateVideoProcessor(uint16_t width,
    uint16_t height) {
  if (width == 0 || height == 0 || d3d11_video_device_ == nullptr)
    return false;

  video_processor_.Release();
  HRESULT hr = d3d11_video_device_->CreateVideoProcessor(video_processors_enum_, 0,
                                                 &video_processor_);
  if (FAILED(hr)) {
    return false;
  }

  return true;
}

bool WebrtcVideoRendererD3D11Impl::RenderFrame(ID3D11Texture2D* texture) {
  if (texture == nullptr || !swap_chain_for_hwnd_.p
      || !d3d11_video_device_ || !d3d11_video_context_)
    return false;

  bool sts = CreateVideoProcessor(width_, height_);
  if (!sts)
    return sts;

  CComPtr<ID3D11Texture2D> current_back_buffer;

  HRESULT hr = swap_chain_for_hwnd_->GetBuffer(
      0, __uuidof(ID3D11Texture2D), (void**)&current_back_buffer.p);
  if (FAILED(hr))
    return false;

  // Create output view and input view
  D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC output_view_desc;
  memset(&output_view_desc, 0, sizeof(output_view_desc));

  output_view_desc.ViewDimension = D3D11_VPOV_DIMENSION_TEXTURE2D;
  output_view_desc.Texture2D.MipSlice = 0;

  hr = d3d11_video_device_->CreateVideoProcessorOutputView(
      current_back_buffer, video_processors_enum_, &output_view_desc,
      &output_view_.p);

  if (FAILED(hr))
    return false;

  D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC input_view_desc;
  memset(&input_view_desc, 0, sizeof(input_view_desc));
  input_view_desc.FourCC = 0;
  input_view_desc.ViewDimension = D3D11_VPIV_DIMENSION_TEXTURE2D;
  input_view_desc.Texture2D.MipSlice = 0;
  input_view_desc.Texture2D.ArraySlice = 0;

  hr = d3d11_video_device_->CreateVideoProcessorInputView(
      texture, video_processors_enum_, &input_view_desc, &input_view_.p);
  if (FAILED(hr))
    return false;

  // Blit NV12 surface to RGB back buffer here.
  RECT rect = {0, 0, width_, height_};
  D3D11_VIDEO_PROCESSOR_STREAM stream;
  memset(&stream, 0, sizeof(stream));
  stream.Enable = true;
  stream.OutputIndex = 0;
  stream.InputFrameOrField = 0;
  stream.PastFrames = 0;
  stream.ppPastSurfaces = nullptr;
  stream.ppFutureSurfaces = nullptr;
  stream.pInputSurface = input_view_;
  stream.ppPastSurfacesRight = nullptr;
  stream.ppFutureSurfacesRight = nullptr;
  stream.pInputSurfaceRight = nullptr;

  d3d11_video_context_->VideoProcessorSetStreamSourceRect(video_processor_, 0,
                                                          true, &rect);
  d3d11_video_context_->VideoProcessorSetStreamFrameFormat(
      video_processor_, 0, D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE);

  hr = d3d11_video_context_->VideoProcessorBlt(
      video_processor_, output_view_, 0, 1, &stream);
  if (FAILED(hr)) {
    return false;
  }

  DXGI_PRESENT_PARAMETERS parameters = {0};
  // TODO: handle device loss, etc. Need to find a way
  // to inform decoder to recreate the d3d11 device/context.
  hr = swap_chain_for_hwnd_->Present1(0, 0, &parameters);
  return true;
}

void WebrtcVideoRendererD3D11Impl::OnFrame(
    const webrtc::VideoFrame& video_frame) {
  if (video_frame.video_frame_buffer()->type() ==
      webrtc::VideoFrameBuffer::Type::kNative) {
    D3D11Handle* native_handle =
        reinterpret_cast<D3D11Handle*>(
            reinterpret_cast<owt::base::NativeHandleBuffer*>(
                video_frame.video_frame_buffer().get())
                ->native_handle());

    if (native_handle == nullptr)
      return;

    ID3D11Device* render_device = native_handle->d3d11_device;
    ID3D11VideoDevice* render_video_device = native_handle->d3d11_video_device;
    ID3D11Texture2D* texture = native_handle->texture;
    ID3D11VideoContext* render_context = native_handle->context;

    uint16_t width = video_frame.video_frame_buffer()->width();
    uint16_t height = video_frame.video_frame_buffer()->height();

    if (width == 0 || height == 0)
      return;

    if (render_device == nullptr || render_video_device == nullptr ||
        wnd_ == nullptr || texture == nullptr)
      return;

    width_ = width;
    height_ = height;
    bool sts = CreateOrUpdateContext(render_device, render_video_device, render_context, width, height);
    if (sts) {
      RenderFrame(texture);
    }
  } else {  // I420 frame passed.
  }
  return;
}
}  // namespace base
}  // namespace owt
