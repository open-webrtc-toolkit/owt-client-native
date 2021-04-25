// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/base/win/videorendererd3d11.h"
#include <array>
#include <cstdio>
#include "rtc_base/logging.h"
#include <system_error>
#include "talk/owt/sdk/base/nativehandlebuffer.h"
#include "talk/owt/sdk/base/win/d3dnativeframe.h"
#include "talk/owt/sdk/include/cpp/owt/base/globalconfiguration.h"
#include "talk/owt/sdk/include/cpp/owt/base/videorendererinterface.h"
#include "third_party/libyuv/include/libyuv/convert.h"
#include "webrtc/api/video/i420_buffer.h"
#include "webrtc/common_video/libyuv/include/webrtc_libyuv.h"

using namespace rtc;

namespace owt {
namespace base {

// Driver specific VPE interface for SR/FRC.
static const GUID GUID_VPE_INTERFACE = {
    0xedd1d4b9,
    0x8659,
    0x4cbc,
    {0xa4, 0xd6, 0x98, 0x31, 0xa2, 0x16, 0x3a, 0xc3}};

#define VPE_FN_SCALING_MODE_PARAM 0x37
#define VPE_FN_MODE_PARAM 0x20
#define VPE_FN_SET_VERSION_PARAM 0x01
#define VPE_FN_SR_SET_PARAM 0x401
#define VPE_FN_SET_CPU_GPU_COPY_PARAM 0x2B

WebrtcVideoRendererD3D11Impl::WebrtcVideoRendererD3D11Impl(HWND wnd)
    : wnd_(wnd), clock_(Clock::GetRealTimeClock()) {
  CreateDXGIFactory(__uuidof(IDXGIFactory2), (void**)(&dxgi_factory_));
  sr_enabled_ = SupportSuperResolution();
}

// The swapchain needs to use window height/width of even number.
bool WebrtcVideoRendererD3D11Impl::GetWindowSizeForSwapChain(int& width, int& height) {
  if (!wnd_ || !IsWindow(wnd_))
    return false;

  RECT rect;
  GetClientRect(wnd_, &rect);
  width = rect.right - rect.left;
  height = rect.bottom - rect.top;

  if (width % 2) {
    width += 1;
  }
  if (height % 2) {
    height += 1;
  }

  return true;
}

void WebrtcVideoRendererD3D11Impl::OnFrame(
    const webrtc::VideoFrame& video_frame) {
  uint16_t width = video_frame.video_frame_buffer()->width();
  uint16_t height = video_frame.video_frame_buffer()->height();
  if (width == 0 || height == 0) {
    RTC_LOG(LS_ERROR) << "Invalid video frame size.";
    return;
  }

  if (!wnd_ || !IsWindow(wnd_) || !IsWindowVisible(wnd_))
    return;

  // Window width here is used to scale down the I420 frame,
  // so we're not rounding it up to even number.
  RECT rect;
  GetClientRect(wnd_, &rect);
  int window_width = rect.right - rect.left;
  int window_height = rect.bottom - rect.top;

  if (video_frame.video_frame_buffer()->type() ==
      webrtc::VideoFrameBuffer::Type::kNative) {
    D3D11ImageHandle* native_handle = reinterpret_cast<D3D11ImageHandle*>(
        reinterpret_cast<owt::base::NativeHandleBuffer*>(
            video_frame.video_frame_buffer().get())
            ->native_handle());

    if (native_handle == nullptr) {
      RTC_LOG(LS_ERROR) << "Invalid video buffer handle.";
      return;
    }

    ID3D11Device* render_device = native_handle->d3d11_device;

    // TODO(johny): Revisit this when capture/encode zero-copy is enabled.
    // the D3D11 device may not be shared by capturer.
    if (!render_device) {
      RTC_LOG(LS_ERROR) << "Invalid d3d11 device passed.";
      return;
    }

    HRESULT hr = S_OK;
    ID3D11Texture2D* texture = native_handle->texture;

    // Validate window
    if (wnd_ && dxgi_factory_ && IsWindow(wnd_) && texture) {
      hr = S_OK;
    } else {
      RTC_LOG(LS_ERROR) << "Invalid window or texture.";
      return;
    }

    RenderNativeHandleFrame(video_frame);
  } else {  // I420 frame passed.
    // First scale down to target window size.
    webrtc::VideoFrame new_frame(video_frame);
    rtc::scoped_refptr<webrtc::I420Buffer> scaled_buffer =
        I420Buffer::Create(window_width, window_height);
    auto i420_buffer = video_frame.video_frame_buffer()->ToI420();
    scaled_buffer->ScaleFrom(*i420_buffer);
    new_frame.set_video_frame_buffer(scaled_buffer);

    RenderI420Frame_DX11(new_frame);
  }
  return;
}

bool WebrtcVideoRendererD3D11Impl::InitD3D11(int width, int height) {
  HRESULT hr = S_OK;
  UINT creation_flags = 0;

  D3D_FEATURE_LEVEL feature_levels_in[] = {D3D_FEATURE_LEVEL_9_1,  D3D_FEATURE_LEVEL_9_2,
                                D3D_FEATURE_LEVEL_9_3,  D3D_FEATURE_LEVEL_10_0,
                                D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_11_0,
                                D3D_FEATURE_LEVEL_11_1};
  D3D_FEATURE_LEVEL feature_levels_out;
  hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
                         creation_flags, feature_levels_in,
                         sizeof(feature_levels_in) / sizeof(D3D_FEATURE_LEVEL),
                         D3D11_SDK_VERSION, &d3d11_device_, &feature_levels_out,
                         &d3d11_device_context_);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "Failed to create D3D11 device for I420 renderer.";
    return false;
  }

  d3d11_raw_inited_ = true;
  hr = d3d11_device_->QueryInterface(__uuidof(ID3D10Multithread),
                                     (void**)(&p_mt));
  hr = p_mt->SetMultithreadProtected(true);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "Failed to enable multi-thread protection.";
    return false;
  }

  return true;
}

bool WebrtcVideoRendererD3D11Impl::InitSwapChain(int width,
    int height, bool reset) {
  if (width <= 0 || height <= 0) {
    RTC_LOG(LS_ERROR) << "Invalid video width for swapchain creation.";
    return false;
  }

  if (!d3d11_device_ || !wnd_) {
    RTC_LOG(LS_ERROR) << "Invalid device for swapchain creation.";
    return false;
  }

  HRESULT hr = S_OK;

  if (!GetWindowSizeForSwapChain(window_width_, window_height_)) {
    RTC_LOG(LS_ERROR) << "Failed to get window size for swapchian.";
    return false;
  }

  webrtc::MutexLock lock(&d3d11_texture_lock_);
  if (swap_chain_for_hwnd_) {
    DXGI_SWAP_CHAIN_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    hr = swap_chain_for_hwnd_->GetDesc(&desc);
    if (FAILED(hr)) {
      RTC_LOG(LS_ERROR) << "Failed to get desc for swapchain";
      return false;
    }

    if (desc.BufferDesc.Width != (unsigned int)window_width_ ||
        desc.BufferDesc.Height != (unsigned int)window_height_) {
      d3d11_device_context_->ClearState();
      d3d11_device_context_->Flush();

      hr = swap_chain_for_hwnd_->ResizeBuffers(0, window_width_, window_height_,
                                              DXGI_FORMAT_UNKNOWN, desc.Flags);
      if (FAILED(hr)) {
        RTC_LOG(LS_ERROR) << "Failed to resize buffer for swapchain.";
        return false;
      }
    } else {
      return true;
    } 
  }

  DXGI_SWAP_CHAIN_DESC1 desc;
  ZeroMemory(&desc, sizeof(DXGI_SWAP_CHAIN_DESC1));
  desc.BufferCount = 2;
  desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
  desc.Height = window_height_;
  desc.Width = window_width_;
  desc.Scaling = DXGI_SCALING_STRETCH;
  desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
  desc.Stereo = false;
  desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

  CComPtr<IDXGIDevice2> dxgi_device;
  hr = d3d11_device_->QueryInterface(__uuidof(IDXGIDevice1),
                                     (void**)&dxgi_device);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "Failed to query dxgi device.";
    return false;
  }

  Microsoft::WRL::ComPtr<IDXGIAdapter> adapter = nullptr;
  Microsoft::WRL::ComPtr<IDXGIFactory2> factory = nullptr;

  hr = dxgi_device->GetAdapter(&adapter);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "Failed to get the adatper.";
    return false;
  }

  hr = adapter->GetParent(IID_PPV_ARGS(&factory));
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "Failed to get dxgi factory.";
    return false;
  }

  d3d11_device_context_->ClearState();
  d3d11_device_context_->Flush();

  if (swap_chain_for_hwnd_)
    swap_chain_for_hwnd_.Release();

  hr = factory->CreateSwapChainForHwnd(d3d11_device_, wnd_, &desc, nullptr, nullptr, &swap_chain_for_hwnd_);
  if (FAILED(hr)) {
    std::string message = std::system_category().message(hr);
    RTC_LOG(LS_ERROR) << "Failed to create swapchain for hwnd." << message;
    return false;
  }

  return true;
}

void WebrtcVideoRendererD3D11Impl::RenderNativeHandleFrame(
    const webrtc::VideoFrame& video_frame) {
  D3D11ImageHandle* native_handle = reinterpret_cast<D3D11ImageHandle*>(
      reinterpret_cast<owt::base::NativeHandleBuffer*>(
          video_frame.video_frame_buffer().get())
          ->native_handle());

  if (native_handle == nullptr)
    return;

  ID3D11Device* render_device = native_handle->d3d11_device;

  if (!render_device) {
    RTC_LOG(LS_ERROR) << "Decoder passed an invalid d3d11 device.";
    return;
  }

  d3d11_device_ = render_device;
  d3d11_texture_ = native_handle->texture;

  if (d3d11_texture_ == nullptr)
    return;

  d3d11_device_->GetImmediateContext(&d3d11_device_context_);
  if (d3d11_device_context_ == nullptr)
    return;

  RenderNV12DXGIMPO(video_frame.width(), video_frame.height());
}

void WebrtcVideoRendererD3D11Impl::RenderNV12DXGIMPO(int width, int height) {
  HRESULT hr = S_OK;
  if (!d3d11_mpo_inited_) {
    bool ret = InitMPO(width, height);
    if (!ret)
      return;
  }

  if (!GetWindowSizeForSwapChain(window_width_, window_height_)) {
    RTC_LOG(LS_ERROR) << "Failed to get window size for swapchain.";
    return;
  }

  if (!d3d11_video_device_) {
    hr = d3d11_device_->QueryInterface(__uuidof(ID3D11VideoDevice),
                                       (void**)&d3d11_video_device_);
    if (FAILED(hr)) {
      RTC_LOG(LS_ERROR)
          << "Failed to get d3d11 video device from d3d11 device.";
      return;
    }
  }

  if (swap_chain_for_hwnd_) {
    DXGI_SWAP_CHAIN_DESC desc;
    hr = swap_chain_for_hwnd_->GetDesc(&desc);
    if (FAILED(hr)) {
      RTC_LOG(LS_ERROR) << "Failed to get the swapchain descriptor.";
      return;
    }

    if (desc.BufferDesc.Width != (unsigned int)window_width_ ||
        desc.BufferDesc.Height != (unsigned int)window_height_) {
      // Hold the lock to avoid rendering when resizing buffer.
      webrtc::MutexLock lock(&d3d11_texture_lock_);
      d3d11_device_context_->ClearState();

      hr = swap_chain_for_hwnd_->ResizeBuffers(0, window_width_, window_height_,
                                               DXGI_FORMAT_UNKNOWN, desc.Flags);
      if (FAILED(hr)) {
        RTC_LOG(LS_ERROR) << "Resizing compositor swapchain failed.";
        return;
      }
    }
  }

  // We are actually not resetting video processor when no input/output size change.
  bool reset = false;

  if (!d3d11_video_context_) {
    hr = d3d11_device_context_->QueryInterface(__uuidof(ID3D11VideoContext),
                                               (void**)&d3d11_video_context_);
    if (FAILED(hr)) {
      RTC_LOG(LS_ERROR) << "Querying d3d11 video context failed.";
      return;
    }
  }

  if (!CreateVideoProcessor(width, height, reset))
    return;

  RenderD3D11Texture(width, height);
}

bool WebrtcVideoRendererD3D11Impl::InitMPO(int width, int height) {
  HRESULT hr = S_OK;
  hr = d3d11_device_->QueryInterface(__uuidof(ID3D11Device2),
                                     (void**)&d3d11_device2_);
  if (FAILED(hr))
    return false;

  hr = d3d11_device_->QueryInterface(&dxgi_device2_);
  if (FAILED(hr))
    return false;

  CComPtr<IDCompositionDesktopDevice> desktop_device;
  hr = DCompositionCreateDevice2(dxgi_device2_, IID_PPV_ARGS(&desktop_device));
  if (FAILED(hr))
    return false;

  hr = desktop_device->QueryInterface(&comp_device2_);
  if (FAILED(hr))
    return false;

  hr = desktop_device->CreateTargetForHwnd(wnd_, false, &comp_target_);

  if (FAILED(hr))
    return false;

  hr = comp_device2_->CreateVisual(&root_visual_);
  if (FAILED(hr))
    return false;

  hr = comp_device2_->CreateVisual(&visual_preview_);
  if (FAILED(hr))
    return false;

  root_visual_->AddVisual(visual_preview_, FALSE, nullptr);

  hr = comp_target_->SetRoot(root_visual_);
  if (FAILED(hr))
    return false;

  hr = root_visual_->SetBitmapInterpolationMode(
      DCOMPOSITION_BITMAP_INTERPOLATION_MODE_LINEAR);
  if (FAILED(hr))
    return false;

  CComPtr<IDXGIAdapter> adapter = nullptr;
  hr = dxgi_device2_->GetAdapter(&adapter);
  if (FAILED(hr))
    return false;

  Microsoft::WRL::ComPtr<IDXGIFactoryMedia> pMediaFactory;
  hr = adapter->GetParent(__uuidof(IDXGIFactoryMedia), (void**)&pMediaFactory);
  if (FAILED(hr))
    return false;


  DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};
  RECT rect;
  GetClientRect(wnd_, &rect);

  if (!GetWindowSizeForSwapChain(window_width_, window_height_)) {
    RTC_LOG(LS_ERROR) << "Failed to get window size for creating swapchain.";
    return false;
  }
  swapChainDesc.Width = window_width_;
  swapChainDesc.Height = window_height_;
  swapChainDesc.Format = DXGI_FORMAT_NV12;
  swapChainDesc.Stereo = false;
  swapChainDesc.SampleDesc.Count = 1;  // Don't use multi-sampling.
  swapChainDesc.SampleDesc.Quality = 0;
  swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDesc.BufferCount = 2;
  swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
  swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_YUV_VIDEO;
  swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
  swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

  // The composition surface handle is only used to create YUV swap chains
  // since CreateSwapChainForComposition can't do that.
  HANDLE handle = INVALID_HANDLE_VALUE;
  hr = DCompositionCreateSurfaceHandle(COMPOSITIONOBJECT_ALL_ACCESS, nullptr,
                                       &handle);
  if (FAILED(hr))
    return false;

  hr = pMediaFactory->CreateSwapChainForCompositionSurfaceHandle(
      dxgi_device2_, handle, &swapChainDesc, nullptr, &swap_chain_for_hwnd_);
  if (FAILED(hr))
    return false;

  hr = root_visual_->SetContent(swap_chain_for_hwnd_);
  if (FAILED(hr))
    return false;

  hr = comp_device2_->Commit();
  if (FAILED(hr))
    return false;

  d3d11_mpo_inited_ = true;
  return true;
}

bool WebrtcVideoRendererD3D11Impl::CreateVideoProcessor(int width,
                                                        int height,
                                                        bool reset) {
  HRESULT hr = S_OK;
  if (width < 0 || height < 0)
    return false;

  if (!GetWindowSizeForSwapChain(window_width_, window_height_))
    return false;

  D3D11_VIDEO_PROCESSOR_CONTENT_DESC content_desc;
  ZeroMemory(&content_desc, sizeof(content_desc));

  if (video_processor_.p && video_processor_enum_.p) {
    hr = video_processor_enum_->GetVideoProcessorContentDesc(&content_desc);
    if (FAILED(hr))
      return false;

    if (content_desc.InputWidth != (unsigned int)width ||
        content_desc.InputHeight != (unsigned int)height ||
        content_desc.OutputWidth != window_width_ ||
        content_desc.OutputHeight != window_height_ || reset) {
      video_processor_enum_.Release();
      video_processor_.Release();
    } else {
      return true;
    }
  }

  ZeroMemory(&content_desc, sizeof(content_desc));
  content_desc.InputFrameFormat = D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE;
  content_desc.InputFrameRate.Numerator = 30;
  content_desc.InputFrameRate.Denominator = 1;
  content_desc.InputWidth = width;
  content_desc.InputHeight = height;
  content_desc.OutputWidth = window_width_;
  content_desc.OutputHeight = window_height_;
  content_desc.OutputFrameRate.Numerator = 30;
  content_desc.OutputFrameRate.Denominator = 1;
  content_desc.Usage = D3D11_VIDEO_USAGE_OPTIMAL_SPEED;

  hr = d3d11_video_device_->CreateVideoProcessorEnumerator(
      &content_desc, &video_processor_enum_);
  if (FAILED(hr))
    return false;

  hr = d3d11_video_device_->CreateVideoProcessor(video_processor_enum_, 0,
                                                 &video_processor_);
  if (FAILED(hr))
    return false;

  return true;
}

void WebrtcVideoRendererD3D11Impl::RenderD3D11Texture(int width, int height) {
  webrtc::MutexLock lock(&d3d11_texture_lock_);
  HRESULT hr = S_OK;

  if (swap_chain_for_hwnd_ == nullptr) {
    RTC_LOG(LS_ERROR) << "Invalid swapchain.";
    return;
  }

  Microsoft::WRL::ComPtr<ID3D11Texture2D> dxgi_back_buffer;
  hr = swap_chain_for_hwnd_->GetBuffer(0, IID_PPV_ARGS(&dxgi_back_buffer));
  if (FAILED(hr)) {
    std::string message = std::system_category().message(hr);
    RTC_LOG(LS_ERROR) << "Failed to get back buffer:" << message;
    return;
  }

  D3D11_TEXTURE2D_DESC back_buffer_desc;
  dxgi_back_buffer->GetDesc(&back_buffer_desc);

  D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC output_view_desc;
  ZeroMemory(&output_view_desc, sizeof(output_view_desc));
  output_view_desc.ViewDimension = D3D11_VPOV_DIMENSION_TEXTURE2D;
  output_view_desc.Texture2D.MipSlice = 0;
  Microsoft::WRL::ComPtr<ID3D11VideoProcessorOutputView> output_view;
  hr = d3d11_video_device_->CreateVideoProcessorOutputView(
      dxgi_back_buffer.Get(), video_processor_enum_, &output_view_desc,
      &output_view);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "Failed to create output view.";
    return;
  }

  D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC input_view_desc;
  ZeroMemory(&input_view_desc, sizeof(input_view_desc));
  input_view_desc.FourCC = 0;
  input_view_desc.ViewDimension = D3D11_VPIV_DIMENSION_TEXTURE2D;
  input_view_desc.Texture2D.MipSlice = 0;
  input_view_desc.Texture2D.ArraySlice = 0;
  Microsoft::WRL::ComPtr<ID3D11VideoProcessorInputView> input_view;
  hr = d3d11_video_device_->CreateVideoProcessorInputView(
      d3d11_texture_, video_processor_enum_, &input_view_desc, &input_view);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "Failed to create input view.";
    return;
  }

  D3D11_VIDEO_PROCESSOR_STREAM stream_data;
  ZeroMemory(&stream_data, sizeof(stream_data));
  stream_data.Enable = TRUE;
  stream_data.OutputIndex = 0;
  stream_data.InputFrameOrField = 0;
  stream_data.PastFrames = 0;
  stream_data.FutureFrames = 0;
  stream_data.ppPastSurfaces = nullptr;
  stream_data.ppFutureSurfaces = nullptr;
  stream_data.pInputSurface = input_view.Get();
  stream_data.ppPastSurfacesRight = nullptr;
  stream_data.ppFutureSurfacesRight = nullptr;
  stream_data.pInputSurfaceRight = nullptr;

  RECT rect = {0};
  rect.right = width;
  rect.bottom = height;
  d3d11_video_context_->VideoProcessorSetStreamSourceRect(video_processor_, 0,
                                                          true, &rect);
  d3d11_video_context_->VideoProcessorSetStreamFrameFormat(
      video_processor_, 0, D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE);

  // Setup VPE for SR.
  if (sr_enabled_) {
    VPE_FUNCTION function_params;
    VPE_VERSION vpe_version = {};
    VPE_MODE vpe_mode = {};
    SR_SCALING_MODE sr_scaling_params = {};
    VPE_SR_PARAMS sr_params = {};
    void* p_ext_data = nullptr;
    UINT data_size = 0;
    const GUID* p_ext_guid = nullptr;

    // Set VPE version
    ZeroMemory(&function_params, sizeof(function_params));
    vpe_version.Version = (UINT)VPE_VERSION_3_0;
    function_params.Function = VPE_FN_SET_VERSION_PARAM;
    function_params.pVPEVersion = &vpe_version;
    p_ext_data = &function_params;
    data_size = sizeof(function_params);
    p_ext_guid = &GUID_VPE_INTERFACE;

    hr = d3d11_video_context_->VideoProcessorSetOutputExtension(
        video_processor_, p_ext_guid, data_size, p_ext_data);
    if (FAILED(hr))
      goto sr_fail;

    // Clear mode
    ZeroMemory(&function_params, sizeof(function_params));
    vpe_mode.Mode = VPE_MODE_NONE;
    function_params.Function = VPE_FN_MODE_PARAM;
    function_params.pVPEMode = &vpe_mode;
    p_ext_data = &function_params;
    data_size = sizeof(function_params);
    p_ext_guid = &GUID_VPE_INTERFACE;
    hr = d3d11_video_context_->VideoProcessorSetOutputExtension(
        video_processor_, p_ext_guid, data_size, p_ext_data);
    if (FAILED(hr))
      goto sr_fail;

    // Set SR parameters
    ZeroMemory(&function_params, sizeof(function_params));
    sr_params.bEnable = true;
    sr_params.SRMode = DEFAULT_SCENARIO_MODE;
    function_params.Function = VPE_FN_SR_SET_PARAM;
    function_params.pSRParams = &sr_params;
    p_ext_data = &function_params;
    data_size = sizeof(function_params);
    p_ext_guid = &GUID_VPE_INTERFACE;
    hr = d3d11_video_context_->VideoProcessorSetOutputExtension(
        video_processor_, p_ext_guid, data_size, p_ext_data);
    if (FAILED(hr))
      goto sr_fail;
  }

sr_fail:
  hr = d3d11_video_context_->VideoProcessorBlt(
      video_processor_, output_view.Get(), 0, 1, &stream_data);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "Failed to blit.";
    return;
  }

  hr = swap_chain_for_hwnd_->Present(1, 0);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "Failed to present the back buffer.";
    return;
  }
}
void WebrtcVideoRendererD3D11Impl::RenderI420Frame_DX11(
    const webrtc::VideoFrame& video_frame) {
  if (!d3d11_raw_inited_ &&
      !InitD3D11(video_frame.width(), video_frame.height())) {
    RTC_LOG(LS_ERROR) << "Failed to init d3d11 device.";
    return;
  }

  if (!InitSwapChain(video_frame.width(), video_frame.height(), false)) {
    RTC_LOG(LS_ERROR) << "Failed to init swapchain.";
    return;
  }
  {
    webrtc::MutexLock lock(&d3d11_texture_lock_);
    if (d3d11_texture_) {
      d3d11_texture_->Release();
      d3d11_texture_ = nullptr;
    }
  }

  if (!CreateStagingTexture(video_frame.width(), video_frame.height())) {
    RTC_LOG(LS_ERROR) << "Failed to create staging texture.";
    return;
  }

  HRESULT hr = S_OK;
  p_mt->Enter();
  D3D11_MAPPED_SUBRESOURCE sub_resource = {0};
  hr = d3d11_device_context_->Map(d3d11_staging_texture_, 0,  D3D11_MAP_READ_WRITE, 0, &sub_resource);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "Failed to map texture.";
    return;
  }

  libyuv::I420ToARGB(video_frame.video_frame_buffer()->GetI420()->DataY(),
                     video_frame.video_frame_buffer()->GetI420()->StrideY(),
                     video_frame.video_frame_buffer()->GetI420()->DataU(),
                     video_frame.video_frame_buffer()->GetI420()->StrideU(),
                     video_frame.video_frame_buffer()->GetI420()->DataV(),
                     video_frame.video_frame_buffer()->GetI420()->StrideV(),
                     static_cast<uint8_t*>(sub_resource.pData),
                     sub_resource.RowPitch,
                     video_frame.video_frame_buffer()->width(),
                     video_frame.video_frame_buffer()->height());
  d3d11_device_context_->Unmap(d3d11_staging_texture_, 0);

  D3D11_TEXTURE2D_DESC desc = {0};
  d3d11_staging_texture_->GetDesc(&desc);
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;
  desc.BindFlags = D3D11_BIND_RENDER_TARGET;
  hr = d3d11_device_->CreateTexture2D(&desc, nullptr, &d3d11_texture_);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "Failed to create render target texture.";
    return;
  }

  {
    webrtc::MutexLock lock(&d3d11_texture_lock_);
    d3d11_device_context_->CopyResource(d3d11_texture_, d3d11_staging_texture_);
    d3d11_texture_->GetDesc(&d3d11_texture_desc_);
  }
  if (!d3d11_video_device_) {
    hr = d3d11_device_->QueryInterface(__uuidof(ID3D11VideoDevice),
                                       (void**)&d3d11_video_device_);
    if (FAILED(hr)) {
      RTC_LOG(LS_ERROR) << "Failed to query d3d11 video device.";
      return;
    }
  }

  if (!d3d11_video_context_) {
    hr = d3d11_device_context_->QueryInterface(__uuidof(ID3D11VideoContext),
                                                 (void**)&d3d11_video_context_);
    if (FAILED(hr)) {
      RTC_LOG(LS_ERROR) << "Failed to get d3d11 video context.";
      return;
    }
  }
  p_mt->Leave();

  if (!CreateVideoProcessor(video_frame.width(), video_frame.height(),
                              false)) {
    RTC_LOG(LS_ERROR) << "Failed to create video processor.";
    return;
  }
  RenderD3D11Texture(video_frame.width(), video_frame.height());
}

bool WebrtcVideoRendererD3D11Impl::CreateStagingTexture(int width, int height) {
  if ((width < 0) || (height < 0))
    return false;
  if (d3d11_staging_texture_) {
    D3D11_TEXTURE2D_DESC desc = {0};
    d3d11_staging_texture_->GetDesc(&desc);
    if (desc.Width != (unsigned int)width ||
        desc.Height != (unsigned int)height) {
      d3d11_staging_texture_->Release();
      d3d11_staging_texture_ = nullptr;
    } else
      return true;
  }
  HRESULT hr = S_OK;
  D3D11_TEXTURE2D_DESC desc = {0};
  desc.Width = (unsigned int)width;
  desc.Height = (unsigned int)height;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.Usage = D3D11_USAGE_STAGING;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
  desc.MiscFlags = 0;
  desc.BindFlags = 0;

  hr = d3d11_device_->CreateTexture2D(&desc, nullptr,
                                            &d3d11_staging_texture_);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "Failed to create staging texture.";
    return false;
  }

  return true;
}

// Checks support for super resolution.
bool WebrtcVideoRendererD3D11Impl::SupportSuperResolution() {
  return GlobalConfiguration::GetVideoSuperResolutionEnabled();
}

}  // namespace base
}  // namespace owt
