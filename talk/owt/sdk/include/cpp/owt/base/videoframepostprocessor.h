// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_VIDEOFRAMEPOSTPROCESSOR_H_
#define OWT_BASE_VIDEOFRAMEPOSTPROCESSOR_H_

#include "third_party/webrtc/api/scoped_refptr.h"
#include "third_party/webrtc/api/video/video_frame_buffer.h"

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

}  // namespace base
}  // namespace owt

#endif  // OWT_IC_VIDEOFRAMEPOSTPROCESSOR_H_
