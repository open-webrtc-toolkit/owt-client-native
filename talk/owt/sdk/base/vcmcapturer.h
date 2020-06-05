// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

/*
 * Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the thirdpartylicense.txt file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef OWT_BASE_VCMCAPTURER_H_
#define OWT_BASE_VCMCAPTURER_H_

#include <memory>
#include <vector>

#include "api/scoped_refptr.h"
#include "modules/video_capture/video_capture.h"
#include "webrtc/rtc_base/thread.h"
#include "talk/owt/sdk/base/cameravideocapturer.h"

// This file is borrowed from webrtc project.
namespace owt {
namespace base {
class VcmCapturer : public CameraVideoCapturer,
                    public rtc::VideoSinkInterface<webrtc::VideoFrame> {
 public:
  static VcmCapturer* Create(size_t width,
                             size_t height,
                             size_t target_fps,
                             size_t capture_device_index);
  virtual ~VcmCapturer();

  void OnFrame(const webrtc::VideoFrame& frame) override;

 private:
  VcmCapturer();
  bool Init(size_t width,
            size_t height,
            size_t target_fps,
            size_t capture_device_index);
  void Destroy();
  rtc::scoped_refptr<webrtc::VideoCaptureModule> CreateDeviceOnVCMThread(
      const char* unique_device_utf8);
  int32_t StartCaptureOnVCMThread(webrtc::VideoCaptureCapability);
  int32_t StopCaptureOnVCMThread();
  void ReleaseOnVCMThread();
  rtc::scoped_refptr<webrtc::VideoCaptureModule> vcm_;
  webrtc::VideoCaptureCapability capability_;
  std::unique_ptr<rtc::Thread> vcm_thread_;
};

}  // namespace base
}  // namespace owt

#endif  // OWT_BASE_VCMCAPTURER_H_
