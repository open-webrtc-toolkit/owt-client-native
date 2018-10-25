/*
 * Intel License
 */

#ifndef OMS_BASE_WEBRTCVIDEORENDERERIMPL_H_
#define OMS_BASE_WEBRTCVIDEORENDERERIMPL_H_

#include "webrtc/api/mediastreaminterface.h"
#include "webrtc/api/video/video_sink_interface.h"
#include "webrtc/api/video/video_frame.h"
#include "talk/oms/sdk/include/cpp/oms/base/videorendererinterface.h"

namespace oms {
namespace base {

class WebrtcVideoRendererImpl
    : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
 public:
  WebrtcVideoRendererImpl(VideoRendererInterface& renderer)
      : renderer_(renderer) {}
  virtual void OnFrame(const webrtc::VideoFrame& frame) override;
  virtual ~WebrtcVideoRendererImpl() {}

 private:
  VideoRendererInterface& renderer_;
};

}
}

#endif  // OMS_BASE_VIDEORENDERERIMPL_H_
