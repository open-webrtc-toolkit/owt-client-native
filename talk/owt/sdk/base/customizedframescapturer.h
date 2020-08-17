// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_CUSTOMIZEDFRAMESCAPTURER_H_
#define OWT_BASE_CUSTOMIZEDFRAMESCAPTURER_H_
#include <string>
#include <vector>
#if defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
#include <memory>
#endif
#include "api/video/video_frame.h"
#include "api/video/video_rotation.h"
#include "api/video/video_sink_interface.h"
#include "media/base/video_adapter.h"
#include "media/base/video_broadcaster.h"
#include "modules/video_capture/video_capture.h"
#include "pc/video_track_source.h"
#include "webrtc/api/video/i420_buffer.h"
#include "webrtc/api/video/video_source_interface.h"
#include "webrtc/modules/video_capture/video_capture_defines.h"
#include "webrtc/modules/video_capture/video_capture_factory.h"
#include "webrtc/rtc_base/bind.h"
#include "webrtc/rtc_base/critical_section.h"
#include "webrtc/rtc_base/stream.h"
#include "webrtc/rtc_base/string_utils.h"
#include "webrtc/rtc_base/thread_annotations.h"
#include "webrtc/rtc_base/constructor_magic.h"
#include "owt/base/framegeneratorinterface.h"
#include "owt/base/videoencoderinterface.h"

namespace owt {
namespace base {

// Simulated video capturer that periodically reads frames from a file.
class CustomizedFramesCapturer : public webrtc::VideoCaptureModule {
 public:
  CustomizedFramesCapturer(
      std::unique_ptr<VideoFrameGeneratorInterface> rawFrameGenerator);
  CustomizedFramesCapturer(int width,
                           int height,
                           int fps,
                           int bitrate_kbps,
                           VideoEncoderInterface* encoder);
  virtual ~CustomizedFramesCapturer();

  // Override virtual methods of parent class VideoCaptureModule.
  virtual void RegisterCaptureDataCallback(
      rtc::VideoSinkInterface<webrtc::VideoFrame>* dataCallback) override;
  virtual void DeRegisterCaptureDataCallback() override;
  virtual int32_t StartCapture(
      const webrtc::VideoCaptureCapability& capability) override;
  virtual int32_t StopCapture() override;
  virtual const char* CurrentDeviceName() const override {
    return "CustomizedCapturer";
  }
  virtual bool CaptureStarted() override;
  virtual int32_t CaptureSettings(webrtc::VideoCaptureCapability& settings) override;
  virtual int32_t SetCaptureRotation(webrtc::VideoRotation rotation) override;
  virtual bool SetApplyRotation(bool enable) override { 
    return false; 
  }
  virtual bool GetApplyRotation() override {
    return false;
  }
 protected:
  // Read a frame and determine how long to wait for the next frame.
  virtual void ReadFrame();
  // Adjust |frame_buffer_|'s capacity to store frame data. |frame_buffer_|'s
  // capacity should be greater or equal to |size|.
  virtual void AdjustFrameBuffer(uint32_t size);

  // Tell generator to cleanup resources. Called by CustomizedFramesThread.
  virtual void CleanupGenerator();

 private:
  class CustomizedFramesThread;  // Forward declaration, defined in .cc.
  int I420DataSize(int height, int stride_y, int stride_u, int stride_v);

  rtc::VideoSinkInterface<webrtc::VideoFrame>* data_callback_;
  std::unique_ptr<VideoFrameGeneratorInterface> frame_generator_;
  VideoEncoderInterface* encoder_;
  std::unique_ptr<CustomizedFramesThread> frames_generator_thread_;
  int width_;
  int height_;
  int fps_;
  int bitrate_kbps_;
  bool capture_started_ = false;
  VideoFrameGeneratorInterface::VideoFrameCodec frame_type_;
  uint32_t frame_buffer_capacity_;
  rtc::scoped_refptr<webrtc::I420Buffer>
      frame_buffer_;  // Reuseable buffer for video frames.

  rtc::CriticalSection lock_;
  rtc::CriticalSection capture_lock_;
  bool quit_ RTC_GUARDED_BY(capture_lock_);
  RTC_DISALLOW_COPY_AND_ASSIGN(CustomizedFramesCapturer);
};
}  // namespace base
}  // namespace owt
#endif  // OWT_BASE_CUSTOMIZEDFRAMESCAPTURER_H_
