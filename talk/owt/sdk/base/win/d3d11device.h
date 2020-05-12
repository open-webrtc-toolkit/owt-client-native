// Copyright (C) <2020> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_WIN_D3D11DEVICE_H_
#define OWT_BASE_WIN_D3D11DEVICE_H_

#include <atlbase.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <windows.h>

#include "talk/owt/sdk/base/win/msdkvideobase.h"

namespace owt {
namespace base {

class RTCD3D11Device {
 public:
  RTCD3D11Device();
  virtual ~RTCD3D11Device();

  mfxStatus Init(mfxHDL window, mfxU32 adapter_idx, ID3D11Device* device);
  mfxStatus Reset();
  mfxStatus SetHandle(mfxHandleType handle_type, mfxHDL handle);
  mfxStatus GetHandle(mfxHandleType handle_type, mfxHDL* handle);
  void Close();

  ID3D11Device* GetD3D11Device() const {
    if (external_d3d11_device_ != nullptr)
      return external_d3d11_device_;
    else
      return d3d11_device_.p;
  }

  ID3D11VideoDevice* GetD3D11VideoDevice() const {
    if (external_d3d11_video_device_ != nullptr)
      return external_d3d11_video_device_;
    else
      return d3d11_video_device_.p;
  }
  ID3D11VideoContext* GetContext() const {
    if (external_d3d11_video_context_ != nullptr)
      return external_d3d11_video_context_;
    else
      return d3d11_video_context_.p;
  }

 private:
  CComPtr<ID3D11Device> d3d11_device_;
  CComPtr<ID3D11DeviceContext> d3d11_device_context_;
  CComQIPtr<ID3D11VideoDevice> d3d11_video_device_;
  CComQIPtr<ID3D11VideoContext> d3d11_video_context_;

  CComQIPtr<IDXGIDevice1> dxgi_device_;
  CComQIPtr<IDXGIAdapter> dxgi_adapter_;
  CComPtr<IDXGIFactory2> dxgi_factory_;
  CComPtr<IDXGISwapChain1> dxgi_swap_chain_;

  HWND window_handle_ = nullptr;

  // External device. Decoder does not need the context.
  ID3D11Device* external_d3d11_device_ = nullptr;
  ID3D11DeviceContext* external_d3d11_device_context_ = nullptr;
  ID3D11VideoDevice* external_d3d11_video_device_ = nullptr;
  ID3D11VideoContext* external_d3d11_video_context_ = nullptr;
};
}  // namespace base
}  // namespace owt
#endif