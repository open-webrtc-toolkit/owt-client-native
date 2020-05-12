// Copyright (C) <2020> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

// This file is borrowed from MSDK sample with modification

#include "talk/owt/sdk/base/win/d3d11device.h"
#include "rtc_base/logging.h"

#define MSDK_D3D11_CHECK(result) \
  {                        \
    if (FAILED(result)) {  \
      RTC_LOG(LS_ERROR) << "Failed with result:" << result; \
      return MFX_ERR_DEVICE_FAILED;  \
    }  \
  }

namespace owt {
namespace base {

RTCD3D11Device::RTCD3D11Device() {
}

RTCD3D11Device::~RTCD3D11Device() {
  Close();
}

mfxStatus RTCD3D11Device::Init(mfxHDL window,
  mfxU32 adapter_num, ID3D11Device* device) {
  window_handle_ = static_cast<HWND>(window);
  HRESULT hr;

  if (device != nullptr) {
    external_d3d11_device_ = device;

    // First get the immediate device context;
    device->GetImmediateContext(&external_d3d11_device_context_);
    if (external_d3d11_device_context_ == nullptr)
      return MFX_ERR_DEVICE_FAILED;

    // Get the ID3D11VideoDevice handle as well.
    hr = device->QueryInterface(__uuidof(ID3D11VideoDevice),
                                (void**)&external_d3d11_video_device_);
    if (FAILED(hr))
      return MFX_ERR_DEVICE_FAILED;

    hr = external_d3d11_device_context_->QueryInterface(__uuidof(ID3D11VideoContext),
                                (void**)&external_d3d11_video_context_);
    if (FAILED(hr))
      return MFX_ERR_DEVICE_FAILED;

    CComQIPtr<ID3D10Multithread> p_mt(external_d3d11_video_context_);
    if (p_mt) {
      p_mt->SetMultithreadProtected(true);
    } else {
      return MFX_ERR_DEVICE_FAILED;
    }
    return MFX_ERR_NONE;
  }

  RTC_LOG(LS_ERROR) << "In d3d11device: Before create the dxgi factory.";
  static D3D_FEATURE_LEVEL feature_levels[] = {
    D3D_FEATURE_LEVEL_11_1,
    D3D_FEATURE_LEVEL_11_0,
    D3D_FEATURE_LEVEL_10_1,
    D3D_FEATURE_LEVEL_10_1
  };

  D3D_FEATURE_LEVEL feature_levels_out;
  MSDK_D3D11_CHECK(CreateDXGIFactory(__uuidof(IDXGIFactory2),
     (void**)(&dxgi_factory_)));
  
  RTC_LOG(LS_ERROR) << "In d3d11device: Before enumerating adapters.";
  MSDK_D3D11_CHECK(dxgi_factory_->EnumAdapters(adapter_num, &dxgi_adapter_));

  RTC_LOG(LS_ERROR) << "In d3d11device: Before D3D11CreateDevice.";
  MSDK_D3D11_CHECK(D3D11CreateDevice(dxgi_adapter_, D3D_DRIVER_TYPE_UNKNOWN, nullptr, 0,
                           feature_levels, MSDK_ARRAY_LEN(feature_levels),
                           D3D11_SDK_VERSION, &d3d11_device_,
                           &feature_levels_out, &d3d11_device_context_));

  dxgi_device_ = d3d11_device_;
  d3d11_video_device_ = d3d11_device_;
  d3d11_video_context_ = d3d11_device_context_;

  if (!dxgi_device_.p || !d3d11_video_device_.p || !d3d11_video_context_.p) {
    RTC_LOG(LS_ERROR) << "d3d11 device or context nullptr";
    return MFX_ERR_NULL_PTR;
  }

  // Turn on multi-threading for the context
  CComQIPtr<ID3D10Multithread> p_mt(d3d11_video_context_);
  if (p_mt) {
    p_mt->SetMultithreadProtected(true);
  } else {
    RTC_LOG(LS_ERROR) << "Failed to turn on multi-threading for the context.";
    return MFX_ERR_DEVICE_FAILED;
  }

  return MFX_ERR_NONE;
}

mfxStatus RTCD3D11Device::Reset() {
  // Currently do nothing.
  return MFX_ERR_NONE;
}

mfxStatus RTCD3D11Device::SetHandle(mfxHandleType handle_type, mfxHDL handle) {
  if (handle != nullptr) {
    window_handle_ = static_cast<HWND>(handle);
    return MFX_ERR_NONE;
  }
  return MFX_ERR_UNSUPPORTED;
}

mfxStatus RTCD3D11Device::GetHandle(mfxHandleType handle_type, mfxHDL* handle) {
  if (MFX_HANDLE_D3D11_DEVICE == handle_type) {
    *handle = external_d3d11_device_ != nullptr ? external_d3d11_device_
                                                : d3d11_device_.p;
    return MFX_ERR_NONE;
  }
  return MFX_ERR_UNSUPPORTED;
}

void RTCD3D11Device::Close() {
  window_handle_ = nullptr;
}

}
}