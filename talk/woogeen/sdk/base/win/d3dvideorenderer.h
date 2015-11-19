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
//In case  receveived data is native buffer that contains a surface as well as dev manager ptr, we will get
// d3d device from dev manager and then render the surface.
//
#include "talk/app/webrtc/mediastreaminterface.h"
#pragma once

class D3DVideoRenderer : public webrtc::VideoRendererInterface{
public:
    D3DVideoRenderer(HWND wnd, int width, int height, webrtc::VideoTrackInterface* track_to_render);
    virtual ~D3DVideoRenderer();

    //VideoRendererInterface implementation
    virtual void SetSize(int width, int height);
    virtual void RenderFrame(const cricket::VideoFrame* frame);

protected:
    HWND wnd_;
    rtc::scoped_refptr<webrtc::VideoTrackInterface> rendered_track_;
    int width_;
    int height_;
};