/*
* Intel License.
*/

//
//C++ SDK specific video renderer implementation. This render will check if native buffer impl ptr is set or not.
//If received buffer is i420 data, will create a new D3D device for rendering the I420 frame.
//In case  receveived data is native buffer that contains a surface as well as dev manager ptr, we will get
// d3d device from dev manager and then render the surface.
//
#ifndef WOOGEEN_BASE_WIN_D3DVIDEORENDERER_H
#define WOOGEEN_BASE_WIN_D3DVIDEORENDERER_H

#include "webrtc/api/mediastreaminterface.h"

namespace woogeen {
namespace base {
class D3DVideoRenderer : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
 public:
  D3DVideoRenderer(HWND wnd, webrtc::VideoTrackInterface* track_to_render);
  virtual ~D3DVideoRenderer();

  // VideoSinkInterface implementation
  virtual void OnFrame(const webrtc::VideoFrame& frame) override;

 private:
  HWND wnd_;
  rtc::scoped_refptr<webrtc::VideoTrackInterface> rendered_track_;
  int width_;
  int height_;
  IDirect3D9Ex* d3d9_;
  IDirect3DDevice9Ex* device_;
};
}
}
#endif