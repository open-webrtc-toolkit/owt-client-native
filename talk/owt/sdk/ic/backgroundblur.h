#ifndef OWT_IC_BACKGROUNDBLUR_H_
#define OWT_IC_BACKGROUNDBLUR_H_

#include <memory>

#include "talk/owt/sdk/base/videoframepostprocessing.h"

namespace owt {
namespace ic {
class SelfieSegmentation;

class BackgroundBlur : public owt::base::VideoFramePostProcessing {
 public:
  BackgroundBlur(int blurRadius = 55);
  ~BackgroundBlur() override;

  rtc::scoped_refptr<webrtc::VideoFrameBuffer> Process(
      const rtc::scoped_refptr<webrtc::VideoFrameBuffer>& buffer) override;

 private:
  std::unique_ptr<SelfieSegmentation> model;
  int blur_radius_;
};

}  // namespace ic
}  // namespace owt

#endif  // OWT_IC_BACKGROUNDBLUR_H_
