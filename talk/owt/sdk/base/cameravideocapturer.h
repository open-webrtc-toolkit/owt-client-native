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

#ifndef OWT_BASE_CAMERACAPTURER_H_
#define OWT_BASE_CAMERACAPTURER_H_

#include <stddef.h>

#include <memory>

#include "api/video/video_frame.h"
#include "api/video/video_source_interface.h"
#include "media/base/video_adapter.h"
#include "media/base/video_broadcaster.h"

// This file is borrowed from webrtc project
namespace owt {
namespace base {

class CameraVideoCapturer : public rtc::VideoSourceInterface<webrtc::VideoFrame> {
 public:
  CameraVideoCapturer();
  ~CameraVideoCapturer() override;

  void AddOrUpdateSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink,
                       const rtc::VideoSinkWants& wants) override;
  void RemoveSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink) override;

 protected:
  void OnFrame(const webrtc::VideoFrame& frame);
  rtc::VideoSinkWants GetSinkWants();

 private:
  void UpdateVideoAdapter();

  rtc::VideoBroadcaster broadcaster_;
  cricket::VideoAdapter video_adapter_;
};
}  // namespace base
}  // namespace owt

#endif  // OWT_BASE_CAMERACAPTURER_H_
