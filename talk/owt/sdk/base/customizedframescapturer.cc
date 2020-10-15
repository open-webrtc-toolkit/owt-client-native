// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "webrtc/common_video/include/video_frame_buffer.h"
#include "webrtc/media/base/video_common.h"
#include "webrtc/rtc_base/logging.h"
#include "webrtc/rtc_base/memory/aligned_malloc.h"
#include "webrtc/rtc_base/thread.h"
#include "webrtc/system_wrappers/include/clock.h"
#include "talk/owt/sdk/base/customizedframescapturer.h"
#include "talk/owt/sdk/base/customizedencoderbufferhandle.h"
#include "talk/owt/sdk/base/nativehandlebuffer.h"

using namespace rtc;
namespace owt {
namespace base {
///////////////////////////////////////////////////////////////////////
// Definition of private class CustomizedFramesThread that periodically
// generates frames.
// TODO: should use rtc::PlatformThread instead as we need to increase
// the priority of the capture thread.
///////////////////////////////////////////////////////////////////////
class CustomizedFramesCapturer::CustomizedFramesThread
    : public rtc::Thread,
      public rtc::MessageHandler {
 public:
  explicit CustomizedFramesThread(CustomizedFramesCapturer* capturer, int fps)
      : rtc::Thread(SocketServer::CreateDefault()),
        capturer_(capturer) {
    finished_ = false;
    waiting_time_ms = 1000 / fps;
  }
  virtual ~CustomizedFramesThread() { Stop(); }
  // Override virtual method of parent Thread. Context: Worker Thread.
  virtual void Run() {
    // Read the first frame and start the message pump. The pump runs until
    // Stop() is called externally or Quit() is called by OnMessage().
    // Before returning, cleanup any thread-sensitive resources.
    if (capturer_) {
      capturer_->ReadFrame();
      rtc::Thread::Current()->Post(RTC_FROM_HERE, this);
      rtc::Thread::Current()->ProcessMessages(kForever);
      capturer_->CleanupGenerator();
      // Clear capturer pointer so any lingerering POSTs back out.
      capturer_ = nullptr;
    }
    rtc::CritScope cs(&crit_);
    finished_ = true;
  }
  // Override virtual method of parent MessageHandler. Context: Worker Thread.
  virtual void OnMessage(rtc::Message* /*pmsg*/) {
    if (capturer_) {
      capturer_->ReadFrame();
      rtc::Thread::Current()->PostDelayed(RTC_FROM_HERE, waiting_time_ms, this);
    } else {
      rtc::Thread::Current()->Quit();
    }
  }
  // Check if Run() is finished.
  bool Finished() const {
    rtc::CritScope cs(&crit_);
    return finished_;
  }

 private:
  CustomizedFramesCapturer* capturer_;
  mutable rtc::CriticalSection crit_;
  bool finished_;
  int waiting_time_ms;
  RTC_DISALLOW_COPY_AND_ASSIGN(CustomizedFramesThread);
};

/////////////////////////////////////////////////////////////////////
// Implementation of class CustomizedFramesCapturer.
/////////////////////////////////////////////////////////////////////
CustomizedFramesCapturer::CustomizedFramesCapturer(
    std::unique_ptr<VideoFrameGeneratorInterface> raw_frameGenerator)
    : frame_generator_(std::move(raw_frameGenerator)),
      encoder_(nullptr),
      width_(frame_generator_->GetWidth()),
      height_(frame_generator_->GetHeight()),
      fps_(frame_generator_->GetFps()),
      bitrate_kbps_(0),
      frame_type_(frame_generator_->GetType()),
      frame_buffer_capacity_(0),
      frame_buffer_(nullptr) {}
CustomizedFramesCapturer::CustomizedFramesCapturer(
    int width,
    int height,
    int fps,
    int bitrate_kbps,
    VideoEncoderInterface* encoder)
    : frame_generator_(nullptr),
      encoder_(encoder),
      width_(width),
      height_(height),
      fps_(fps),
      bitrate_kbps_(bitrate_kbps),
      frame_buffer_capacity_(0),
      frame_buffer_(nullptr) {}
CustomizedFramesCapturer::~CustomizedFramesCapturer() {
  DeRegisterCaptureDataCallback();
  StopCapture();
  frame_generator_.reset(nullptr);
  // Encoder is created by app. And needs to be freed by
  // application. mark it to nullptr to avoid ReadFrame
  // passing native buffer to stack.
  encoder_ = nullptr;
}

void CustomizedFramesCapturer::RegisterCaptureDataCallback(
    rtc::VideoSinkInterface<webrtc::VideoFrame>* dataCallback){
  rtc::CritScope lock(&lock_);
  data_callback_ = dataCallback;
}

void CustomizedFramesCapturer::DeRegisterCaptureDataCallback() {
  rtc::CritScope lock(&lock_);
  data_callback_ = nullptr;
}

int32_t CustomizedFramesCapturer::StartCapture(
    const webrtc::VideoCaptureCapability& capability) {
  rtc::CritScope lock(&capture_lock_);
  if (capture_started_)
    return 0;
  if (!frames_generator_thread_) {
    quit_ = false;
    frames_generator_thread_.reset(new CustomizedFramesThread(this, fps_));

    bool ret = frames_generator_thread_->Start();
    if (!ret) {
      frames_generator_thread_.reset();
      return -1;
    }
  }
  capture_started_ = true;
  return 0;
}

int32_t CustomizedFramesCapturer::StopCapture() {
  rtc::CritScope lock(&capture_lock_);
  if (frames_generator_thread_) {
    {
      quit_ = true;
      frames_generator_thread_->Quit();
      frames_generator_thread_.reset();
    }
  }
  capture_started_ = false;
  return 0;
}

void CustomizedFramesCapturer::CleanupGenerator() {
  if (frame_generator_) {
    frame_generator_->Cleanup();
  }
}

bool CustomizedFramesCapturer::CaptureStarted() {
  return capture_started_;
}

int32_t CustomizedFramesCapturer::CaptureSettings(
    webrtc::VideoCaptureCapability& settings) {
  settings.width = width_;
  settings.height = height_;
  settings.maxFPS = fps_;
  settings.videoType = webrtc::VideoType::kI420;

  return 0;
}

int32_t CustomizedFramesCapturer::SetCaptureRotation(
    webrtc::VideoRotation rotation) {
  // Not implemented.
  return 0;
}

int CustomizedFramesCapturer::I420DataSize(int height,
                                           int stride_y,
                                           int stride_u,
                                           int stride_v) {
  return stride_y * height + (stride_u + stride_v) * ((height + 1) / 2);
}

void CustomizedFramesCapturer::AdjustFrameBuffer(uint32_t size) {
  if (size > frame_buffer_capacity_ || !frame_buffer_) {
    RTC_LOG(LS_VERBOSE) << "Allocate new memory for frame buffer.";
    width_ = frame_generator_->GetWidth();
    height_ = frame_generator_->GetHeight();
    int stride_y = width_;
    int stride_uv = (width_ + 1) / 2;
    frame_buffer_ = webrtc::I420Buffer::Create(width_, height_, stride_y,
                                               stride_uv, stride_uv);
    frame_buffer_capacity_ =
        I420DataSize(height_, stride_y, stride_uv, stride_uv);
    if (frame_buffer_capacity_ < size) {
      RTC_LOG(LS_ERROR) << "User provides invalid data size. Expected size: "
                        << frame_buffer_capacity_ << ", user wants: " << size;
    }
  }
}

// Executed in the context of CustomizedFramesThread.
void CustomizedFramesCapturer::ReadFrame() {
  // Signal the previously read frame to downstream in worker_thread.
  rtc::CritScope lock(&lock_);
  if (!data_callback_)
    return;
  if (frame_generator_ != nullptr) {
    auto frame_size = frame_generator_->GetNextFrameSize();
    AdjustFrameBuffer(frame_size);
    if (frame_generator_->GenerateNextFrame(frame_buffer_->MutableDataY(),
                                            frame_buffer_capacity_) !=
        frame_size) {
      RTC_DCHECK(false);
      RTC_LOG(LS_ERROR) << "Failed to get video frame.";
      return;
    }

    webrtc::VideoFrame capture_frame =
        webrtc::VideoFrame::Builder()
            .set_video_frame_buffer(frame_buffer_)
            .set_timestamp_rtp(0)
            .set_timestamp_ms(rtc::TimeMillis())
            .set_rotation(webrtc::kVideoRotation_0)
            .build();

    capture_frame.set_ntp_time_ms(0);
    data_callback_->OnFrame(capture_frame);
  } else if (encoder_ != nullptr) {  // video encoder interface used. Pass the
                                     // encoder information.
    CustomizedEncoderBufferHandle* encoder_context =
        new CustomizedEncoderBufferHandle;
    encoder_context->encoder = encoder_;
    encoder_context->width = width_;
    encoder_context->height = height_;
    encoder_context->fps = fps_;
    encoder_context->bitrate_kbps = bitrate_kbps_;
    rtc::scoped_refptr<owt::base::EncodedFrameBuffer> buffer =
        new rtc::RefCountedObject<owt::base::EncodedFrameBuffer>(
            encoder_context);

    webrtc::VideoFrame pending_frame =
        webrtc::VideoFrame::Builder()
            .set_video_frame_buffer(buffer)
            .set_timestamp_rtp(0)
            .set_timestamp_ms(rtc::TimeMillis())
            .set_rotation(webrtc::kVideoRotation_0)
            .build();

    pending_frame.set_ntp_time_ms(0);
    data_callback_->OnFrame(pending_frame);
  }
}
}  // namespace base
}  // namespace owt
