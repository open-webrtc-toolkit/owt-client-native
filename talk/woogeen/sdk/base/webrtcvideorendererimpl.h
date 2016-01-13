/*
 * Intel License
 */

#ifndef WOOGEEN_BASE_WEBRTCVIDEORENDERERIMPL_H_
#define WOOGEEN_BASE_WEBRTCVIDEORENDERERIMPL_H_

#include "talk/app/webrtc/mediastreaminterface.h"
#include "talk/media/base/videoframe.h"
#include "talk/woogeen/sdk/include/cpp/woogeen/base/videorendererinterface.h"

namespace woogeen {
namespace base {
class WebrtcVideoRendererARGBImpl : public webrtc::VideoRendererInterface {
 public:
  WebrtcVideoRendererARGBImpl(VideoRendererARGBInterface& renderer)
      : renderer_(renderer) {}
  virtual void RenderFrame(const cricket::VideoFrame* frame) override;
  virtual ~WebrtcVideoRendererARGBImpl() {}

 private:
  VideoRendererARGBInterface& renderer_;
};
}
}

#endif  // WOOGEEN_BASE_VIDEORENDERERIMPL_H_
