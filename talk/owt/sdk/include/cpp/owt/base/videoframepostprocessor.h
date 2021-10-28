// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_VIDEOFRAMEPOSTPROCESSING_H_
#define OWT_BASE_VIDEOFRAMEPOSTPROCESSING_H_

#include "rtc_base/logging.h"
#include "api/scoped_refptr.h"
#include "api/video/video_frame_buffer.h"
#include "talk/owt/sdk/base/sharedobjectpointer.h"

namespace owt {
namespace base {

class VideoFramePostProcessor {
 public:
  virtual ~VideoFramePostProcessor() {}

  virtual bool SetParameter(const std::string& key, const std::string& value) {
    return false;
  }

  virtual rtc::scoped_refptr<webrtc::VideoFrameBuffer> Process(
      const rtc::scoped_refptr<webrtc::VideoFrameBuffer>& buffer) = 0;
};

template <>
struct SOTrait<VideoFramePostProcessor> {
  static constexpr auto name = "PostProcessor";
};

}  // namespace base
}  // namespace owt

#endif  // OWT_IC_VIDEOFRAMEPOSTPROCESSING_H_
