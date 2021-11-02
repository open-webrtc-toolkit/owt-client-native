// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_IC_BACKGROUNDBLUR_H_
#define OWT_IC_BACKGROUNDBLUR_H_

#include <memory>

#include "talk/owt/sdk/include/cpp/owt/base/videoframepostprocessor.h"
#include "talk/owt/sdk/ic/selfiesegmentationmodel.h"

namespace owt {
namespace ic {

class BackgroundBlur : public owt::base::VideoFramePostProcessor {
 public:
  BackgroundBlur(InferenceEngine::Core& core);
  ~BackgroundBlur() override = default;

  bool SetParameter(const std::string& key, const std::string& value) override;

  rtc::scoped_refptr<webrtc::VideoFrameBuffer> Process(
      const rtc::scoped_refptr<webrtc::VideoFrameBuffer>& buffer) override;

 private:
  SelfieSegmentationModel model;
  int blur_radius_ = 55;
};

}  // namespace ic
}  // namespace owt

#endif  // OWT_IC_BACKGROUNDBLUR_H_
