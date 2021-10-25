// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_VIDEOFRAMEPOSTPROCESSING_H_
#define OWT_BASE_VIDEOFRAMEPOSTPROCESSING_H_

#include "api/scoped_refptr.h"
#include "api/video/video_frame_buffer.h"

namespace owt {
namespace base {

class VideoFramePostProcessor {
 public:
  virtual ~VideoFramePostProcessor() = default;

  virtual rtc::scoped_refptr<webrtc::VideoFrameBuffer> Process(
      const rtc::scoped_refptr<webrtc::VideoFrameBuffer>& buffer) = 0;

  virtual void Release() noexcept { delete this; }
};

}  // namespace base
}  // namespace owt

#endif  // OWT_IC_VIDEOFRAMEPOSTPROCESSING_H_