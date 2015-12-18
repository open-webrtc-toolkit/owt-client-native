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

//d3dnativeframe implementation.
#include "webrtc/base/checks.h"
#include "talk/woogeen/sdk/base/win/d3dnativeframe.h"

NativeD3DSurfaceHandleImpl::NativeD3DSurfaceHandleImpl() :surface_(NULL), dev_manager_(NULL){}

void NativeD3DSurfaceHandleImpl::SetD3DSurfaceObject(IDirect3DDeviceManager9* dev_manager, IDirect3DSurface9* surface){
    dev_manager_ = dev_manager;
    surface_ = surface;
}

D3DNativeHandleBuffer::D3DNativeHandleBuffer(void* native_handle, int width, int height, int data_size)
    :webrtc::NativeHandleBuffer(native_handle_, width, height, data_size){}

rtc::scoped_refptr<webrtc::VideoFrameBuffer> D3DNativeHandleBuffer::NativeToI420Buffer(){
    RTC_NOTREACHED();
    return nullptr;
}


