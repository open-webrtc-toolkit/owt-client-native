/*
* libjingle
* Copyright 2012 Google Inc.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*  1. Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*  2. Redistributions in binary form must reproduce the above copyright notice,
*     this list of conditions and the following disclaimer in the documentation
*     and/or other materials provided with the distribution.
*  3. The name of the author may not be used to endorse or promote products
*     derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
* EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//
//C++ SDK specific video renderer implementation. This render will check if native buffer impl ptr is set or not.
//If received buffer is i420 data, will create a new D3D device for rendering the I420 frame.
//
#include "talk/woogeen/sdk/base/win/d3dvideorenderer.h"
#include "talk/woogeen/sdk/base/win/d3dnativeframe.h"

#include "webrtc/media/base/videoframe.h"

#include <d3d9.h>

D3DVideoRenderer::D3DVideoRenderer(HWND wnd, int width, int height, webrtc::VideoTrackInterface* track_to_render)
  : wnd_(wnd), width_(width), height_(height), rendered_track_(track_to_render){
    rtc::VideoSinkWants wants;
    rendered_track_->AddOrUpdateSink(this, wants);
}

D3DVideoRenderer::~D3DVideoRenderer(){
    rendered_track_->RemoveSink(this);
}

void D3DVideoRenderer::SetSize(int width, int height){
    width_ = width;
    height_ = height;
}

//
//TODO: if the rotation is specified for a D3D surface, we need to
//create extra texture for rotation. This is not yet implemented.
//
void D3DVideoRenderer::OnFrame(const webrtc::VideoFrame& video_frame){
    //Do we need to Lock the renderframe call? since we have the device lock here it seems
    //no neccessary.
    SetSize(video_frame.width(), video_frame.height());

    if (video_frame.video_frame_buffer()->native_handle() == nullptr) {  // We're not handling DXVA buffer
      auto frame = webrtc::I420Buffer::Rotate(video_frame.video_frame_buffer(), video_frame.rotation());
      NativeD3DSurfaceHandleImpl* nativeHandle =
          reinterpret_cast<NativeD3DSurfaceHandleImpl*>(frame->native_handle());
      IDirect3DDeviceManager9* dev_manager = nativeHandle->GetD3DManager();
      IDirect3DSurface9* surface = nativeHandle->GetSurface();

      if (dev_manager == nullptr || surface == nullptr)
        return;

      IDirect3DDevice9* pDevice;
      HANDLE hDevice = 0;
      HRESULT hr = dev_manager->OpenDeviceHandle(&hDevice);
      if (FAILED(hr)) {
        LOG(LS_ERROR) << "Failed to open the d3d device handle";
        return;
        }

        hr = dev_manager->LockDevice(hDevice, &pDevice, FALSE);
        if (FAILED(hr)){
            LOG(LS_ERROR) << "Failed to lock device";
            dev_manager->CloseDeviceHandle(hDevice);
            return;
        }
        pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);

        IDirect3DSurface9* back_buffer;
        hr = pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &back_buffer);
        if (FAILED(hr)){
            LOG(LS_ERROR) << "Failed to get backbuffer";
            dev_manager->UnlockDevice(hDevice, FALSE);
            dev_manager->CloseDeviceHandle(hDevice);
            return;
        }
        //We don't need to lock the back_buffer as we're not directly copying frame to
        //it.
        hr = pDevice->StretchRect(surface, NULL, back_buffer, NULL, D3DTEXF_NONE);
        if (FAILED(hr)){
            LOG(LS_ERROR) << "Failed to blit the video frame";
            dev_manager->UnlockDevice(hDevice, FALSE);
            dev_manager->CloseDeviceHandle(hDevice);
            return;
        }

        pDevice->Present(NULL, NULL, NULL, NULL);
        surface->Release();
        dev_manager->UnlockDevice(hDevice, FALSE);

        dev_manager->CloseDeviceHandle(hDevice);
    }

    return;
}
