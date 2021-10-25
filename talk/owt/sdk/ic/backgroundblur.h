// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_IC_BACKGROUNDBLUR_H_
#define OWT_IC_BACKGROUNDBLUR_H_

#include <memory>

#include "talk/owt/sdk/base/videoframepostprocessing.h"

namespace owt {
namespace ic {
class SelfieSegmentation;

class BackgroundBlur : public owt::base::VideoFramePostProcessor {
 public:
  BackgroundBlur();
  ~BackgroundBlur() override = default;

  rtc::scoped_refptr<webrtc::VideoFrameBuffer> Process(
      const rtc::scoped_refptr<webrtc::VideoFrameBuffer>& buffer) override;

 private:
  std::unique_ptr<SelfieSegmentation> model;
  int blur_radius_ = 55;
};

}  // namespace ic
}  // namespace owt

#endif  // OWT_IC_BACKGROUNDBLUR_H_
