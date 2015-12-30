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
class WebrtcVideoRendererRGBImpl : public webrtc::VideoRendererInterface {
 public:
  WebrtcVideoRendererRGBImpl(VideoRendererRGBInterface& renderer)
      : renderer_(renderer) {}
  virtual void RenderFrame(const cricket::VideoFrame* frame) override;
  virtual ~WebrtcVideoRendererRGBImpl() {}

 private:
  VideoRendererRGBInterface& renderer_;
};
}
}

#endif  // WOOGEEN_BASE_VIDEORENDERERIMPL_H_
