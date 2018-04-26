/*
 * Intel License
 */

#ifndef ICS_BASE_WEBRTCVIDEORENDERERIMPL_H_
#define ICS_BASE_WEBRTCVIDEORENDERERIMPL_H_

#include "webrtc/api/mediastreaminterface.h"
#include "webrtc/api/videosinkinterface.h"
#include "webrtc/api/video/video_frame.h"
#include "talk/ics/sdk/include/cpp/ics/base/videorendererinterface.h"

namespace ics {
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

#endif  // ICS_BASE_VIDEORENDERERIMPL_H_
