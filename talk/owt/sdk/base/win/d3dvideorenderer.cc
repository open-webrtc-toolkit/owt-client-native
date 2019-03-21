// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include <atlbase.h>
#include <codecapi.h>
#include <combaseapi.h>
#include <d3d9.h>
#include <dxva2api.h>
#include <Windows.h>
#include "talk/owt/sdk/base/win/d3dvideorenderer.h"
#include "talk/owt/sdk/base/win/d3dnativeframe.h"
#include "webrtc/media/base/videoframe.h"

namespace owt {
namespace base {

D3DVideoRenderer::D3DVideoRenderer(HWND wnd,
                                   webrtc::VideoTrackInterface* track_to_render)
    : wnd_(wnd), rendered_track_(track_to_render) {
  rtc::VideoSinkWants wants;
  rendered_track_->AddOrUpdateSink(this, wants);
}

D3DVideoRenderer::~D3DVideoRenderer() {
  rendered_track_->RemoveSink(this);
}

//
// TODO: if the rotation is specified for a D3D surface, we need to
// create extra texture for rotation. This is not yet implemented.
//
void D3DVideoRenderer::OnFrame(const webrtc::VideoFrame& video_frame) {
  // Do we need to Lock the renderframe call? since we have the device lock here
  // it seems
  // no neccessary.
  if (video_frame.video_frame_buffer()->native_handle() !=
      nullptr) {  // We're not handling DXVA buffer
    // auto frame = webrtc::I420Buffer::Rotate(video_frame.video_frame_buffer(),
    // video_frame.rotation());
    NativeD3DSurfaceHandle* nativeHandle =
        reinterpret_cast<NativeD3DSurfaceHandle*>(
            video_frame.video_frame_buffer()->native_handle());
    IDirect3DDeviceManager9* dev_manager = nativeHandle->dev_manager_;
    IDirect3DSurface9* surface = nativeHandle->surface_;
    // int codec_width = nativeHandle->width_;
    // int codec_height = nativeHandle->height_;
    UINT dev_manager_reset_token = nativeHandle->dev_manager_reset_token_;

    // dev manager is created by decoder. MUST NOT be nullptr.
    if (dev_manager == nullptr)
      return;

    // a null surface indicates the decoder requesting D3D9 device from
    // renderer.
    // create it here using the window
    if (surface == nullptr) {
      if (!IsWindow(wnd_))  // Simple validity check in case the window is
                            // already destroyed.
        return;

      RECT r;
      HRESULT hr;

      hr = Direct3DCreate9Ex(D3D_SDK_VERSION, &d3d9_);
      if (FAILED(hr)) {
        LOG(LS_ERROR) << "Failed to create d3d9 context for device creation";
        return;
      }

      D3DPRESENT_PARAMETERS present_params = {0};

      GetClientRect((HWND)wnd_, &r);
      present_params.BackBufferWidth = r.right - r.left;
      present_params.BackBufferHeight = r.bottom - r.top;
      present_params.BackBufferFormat =
          D3DFMT_X8R8G8B8;  // Only apply this if we're rendering full screen
      present_params.BackBufferCount = 1;
      present_params.SwapEffect = D3DSWAPEFFECT_DISCARD;
      present_params.hDeviceWindow = wnd_;
      // present_params.AutoDepthStencilFormat = D3DFMT_D24S8;
      present_params.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
      present_params.Flags =
          D3DPRESENTFLAG_LOCKABLE_BACKBUFFER | D3DPRESENTFLAG_VIDEO;
      present_params.Windowed = TRUE;
      present_params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

      hr = d3d9_->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, wnd_,
                                 D3DCREATE_SOFTWARE_VERTEXPROCESSING |
                                     D3DCREATE_FPU_PRESERVE |
                                     D3DCREATE_MULTITHREADED,
                                 &present_params, NULL, &device_);

      if (FAILED(hr)) {
        LOG(LS_ERROR) << "Failed to create d3d9 device";
        return;
      }

      hr = dev_manager->ResetDevice(device_, dev_manager_reset_token);
      if (FAILED(hr)) {
        LOG(LS_ERROR) << "Failed to set device to device manager";
        return;
      }
      IDirect3DQuery9* query_;
      hr = device_->CreateQuery(D3DQUERYTYPE_EVENT, &query_);
      if (FAILED(hr)) {
        LOG(LS_ERROR) << "Failed to create query";
        return;
      }
      hr = query_->Issue(D3DISSUE_END);
      // Finish the creation. No render this time. Return directly.
      return;
    }
    IDirect3DDevice9* pDevice;
    HANDLE hDevice = 0;
    HRESULT hr = dev_manager->OpenDeviceHandle(&hDevice);
    if (FAILED(hr)) {
      LOG(LS_ERROR) << "Failed to open the d3d device handle";
      return;
    }

    // Our renderer does not need to lock the device.
    hr = dev_manager->LockDevice(hDevice, &pDevice, FALSE);
    if (FAILED(hr)) {
      LOG(LS_ERROR) << "Failed to lock device";
      dev_manager->CloseDeviceHandle(hDevice);
      return;
    }
    pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);
    pDevice->Present(NULL, NULL, NULL, NULL);

    dev_manager->UnlockDevice(hDevice, FALSE);

    dev_manager->CloseDeviceHandle(hDevice);
  }

  return;
}

}
}
