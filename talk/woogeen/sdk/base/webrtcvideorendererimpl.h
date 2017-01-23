/*
 * Intel License
 */

#ifndef WOOGEEN_BASE_WEBRTCVIDEORENDERERIMPL_H_
#define WOOGEEN_BASE_WEBRTCVIDEORENDERERIMPL_H_

#include "webrtc/api/mediastreaminterface.h"
#include "webrtc/media/base/videosinkinterface.h"
#include "webrtc/media/base/videoframe.h"
#include "talk/woogeen/sdk/include/cpp/woogeen/base/videorendererinterface.h"

namespace woogeen {
namespace base {

class WebrtcVideoRendererARGBImpl
    : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
 public:
  WebrtcVideoRendererARGBImpl(VideoRendererARGBInterface& renderer)
      : renderer_(renderer) {}
  virtual void OnFrame(const webrtc::VideoFrame& frame) override;
  virtual ~WebrtcVideoRendererARGBImpl() {}

 private:
  VideoRendererARGBInterface& renderer_;
};
}
}

#endif  // WOOGEEN_BASE_VIDEORENDERERIMPL_H_
