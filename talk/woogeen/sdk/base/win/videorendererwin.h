/*
 * Intel License
 */
#ifndef WOOGEEN_BASE_WIN_VIDEORENDERER_WIN_H
#define WOOGEEN_BASE_WIN_VIDEORENDERER_WIN_H

#include "webrtc/api/mediastreaminterface.h"
#include "webrtc/media/base/videosinkinterface.h"
#include "webrtc/media/base/videoframe.h"

namespace woogeen {
namespace base {

class WebrtcVideoRendererD3D9Impl
    : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
 public:
  WebrtcVideoRendererD3D9Impl(HWND wnd) : wnd_(wnd), first_frame_(true) {}
  virtual void OnFrame(const webrtc::VideoFrame& frame) override;
  virtual ~WebrtcVideoRendererD3D9Impl(){};

 private:
  HWND wnd_;
  bool first_frame_;
};
}
}
#endif  // WOOGEEN_BASE_WIN_VIDEORENDERER_WIN_H
