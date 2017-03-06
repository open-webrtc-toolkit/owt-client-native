/*
 * Intel License
 */
#include <atlbase.h>
#include <codecapi.h>
#include <combaseapi.h>
#include <d3d9.h>
#include <dxva2api.h>
#include <Windows.h>
#include "talk/woogeen/sdk/base/win/videorendererwin.h"
#include "talk/woogeen/sdk/base/win/d3dnativeframe.h"
#include "webrtc/common_video/libyuv/include/webrtc_libyuv.h"
#include "webrtc/media/base/videocommon.h"

namespace woogeen {
namespace base {

void WebrtcVideoRendererD3D9Impl::OnFrame(
    const webrtc::VideoFrame& video_frame) {
  // Do we need to Lock the renderframe call? since we have the device lock here
  // it seems no neccessary.
  if (video_frame.video_frame_buffer()->native_handle() !=
      nullptr) {  // We're handling DXVA buffer
    NativeD3DSurfaceHandle* nativeHandle =
        reinterpret_cast<NativeD3DSurfaceHandle*>(
            video_frame.video_frame_buffer()->native_handle());
    IDirect3DDeviceManager9* dev_manager = nativeHandle->dev_manager_;
    IDirect3DSurface9* surface = nativeHandle->surface_;

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
    CComPtr<IDirect3DSurface9> back_buffer;
    hr = pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &back_buffer);
    if (FAILED(hr)) {
      return;
    }

    if (surface != nullptr) {
      hr = pDevice->StretchRect(surface, nullptr, back_buffer, NULL,
                                D3DTEXF_NONE);
      if (FAILED(hr)) {
        return;
      }
    }
    pDevice->Present(NULL, NULL, wnd_, NULL);

    dev_manager->UnlockDevice(hDevice, FALSE);
    dev_manager->CloseDeviceHandle(hDevice);

    // Done with the native handle.
    delete nativeHandle;
    nativeHandle = nullptr;
  }

  return;
}
}
}
