// Copyright (C) <2020> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

// This file is borrowed from MSDK sample with modification

#include "talk/owt/sdk/base/win/d3d11device.h"

#define MSDK_D3D11_CHECK(result) \
  {                        \
    if (FAILED(result)) {  \
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
  mfxU32 adapter_num) {
  window_handle_ = static_cast<HWND>(window);

  static D3D_FEATURE_LEVEL feature_levels[] = {
    D3D_FEATURE_LEVEL_11_1,
    D3D_FEATURE_LEVEL_11_0,
    D3D_FEATURE_LEVEL_10_1,
    D3D_FEATURE_LEVEL_10_1
  };

  D3D_FEATURE_LEVEL feature_levels_out;
  MSDK_D3D11_CHECK(CreateDXGIFactory(__uuidof(IDXGIFactory2),
     (void**)(&dxgi_factory_)));

  MSDK_D3D11_CHECK(dxgi_factory_->EnumAdapters(adapter_num, &dxgi_adapter_));

  MSDK_D3D11_CHECK(D3D11CreateDevice(dxgi_adapter_, D3D_DRIVER_TYPE_UNKNOWN, nullptr, 0,
                           feature_levels, MSDK_ARRAY_LEN(feature_levels),
                           D3D11_SDK_VERSION, &d3d11_device_,
                           &feature_levels_out, &d3d11_device_context_));

  dxgi_device_ = d3d11_device_;
  d3d11_video_device_ = d3d11_device_;
  d3d11_video_context_ = d3d11_device_context_;

  if (!dxgi_device_.p || !d3d11_video_device_.p || !d3d11_video_context_.p) {
    return MFX_ERR_NULL_PTR;
  }

  // Turn on multi-threading for the context
  CComQIPtr<ID3D10Multithread> p_mt(d3d11_video_context_);
  if (p_mt) {
    p_mt->SetMultithreadProtected(true);
  } else {
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
    *handle = d3d11_device_.p;
    return MFX_ERR_NONE;
  }
  return MFX_ERR_UNSUPPORTED;
}

void RTCD3D11Device::Close() {
  window_handle_ = nullptr;
}

}
}