/*
 * libjingle
 * Copyright 2004--2014 Google Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <iostream>
#include "webrtc/base/bytebuffer.h"
#include "webrtc/base/criticalsection.h"
#include "webrtc/base/logging.h"
#include "webrtc/base/thread.h"
#include "webrtc/system_wrappers/interface/clock.h"
#include "webrtc/system_wrappers/interface/tick_util.h"
#include "talk/woogeen/sdk/base/customizedframescapturer.h"

namespace woogeen {
namespace base {

///////////////////////////////////////////////////////////////////////
// Definition of private class CustomizedFramesThread that periodically generates
// frames.
///////////////////////////////////////////////////////////////////////
class CustomizedFramesCapturer::CustomizedFramesThread
    : public rtc::Thread, public rtc::MessageHandler {
 public:
  explicit CustomizedFramesThread(CustomizedFramesCapturer* capturer, int fps)
      : capturer_(capturer),
        finished_(false) {
    waiting_time_ms = 1000/fps;
  }

  virtual ~CustomizedFramesThread() {
    Stop();
  }

  // Override virtual method of parent Thread. Context: Worker Thread.
  virtual void Run() {
    // Read the first frame and start the message pump. The pump runs until
    // Stop() is called externally or Quit() is called by OnMessage().
    if (capturer_) {
      capturer_->ReadFrame();
      rtc::Thread::Current()->Post(this);
      rtc::Thread::Current()->ProcessMessages(kForever);
    }

    rtc::CritScope cs(&crit_);
    finished_ = true;
  }

  // Override virtual method of parent MessageHandler. Context: Worker Thread.
  virtual void OnMessage(rtc::Message* /*pmsg*/) {
    if (capturer_) {
      capturer_->ReadFrame();
      rtc::Thread::Current()->PostDelayed(waiting_time_ms, this);
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

const char* CustomizedFramesCapturer::kRawFrameDeviceName = "CustomizedFramesGenerator";

CustomizedFramesCapturer::CustomizedFramesCapturer(FrameGeneratorInterface* rawFrameGenerator) {
  frame_generator_ = rawFrameGenerator;
  width_ = frame_generator_->GetWidth();
  height_ = frame_generator_->GetHeight();
  fps_ = frame_generator_->GetFps();
  frame_type_ = frame_generator_->GetType();
  frames_generator_thread = NULL;
  async_invoker_ = nullptr;
}

CustomizedFramesCapturer::~CustomizedFramesCapturer() {
  Stop();
  frame_generator_ = NULL;
  delete[] static_cast<char*>(captured_frame_.data);
}

void CustomizedFramesCapturer::Init() {
  // Enumerate the supported formats. We have only one supported format.
  if (frame_type_ == FrameGeneratorInterface::I420) {
     captured_frame_.fourcc = FOURCC_I420;
  } else if (frame_type_ == FrameGeneratorInterface::VP8) {
     captured_frame_.fourcc = FOURCC_VP80;
  } else if (frame_type_ == FrameGeneratorInterface::H264) {
     captured_frame_.fourcc = FOURCC_H264;
  }
  captured_frame_.pixel_height = 1;
  captured_frame_.pixel_width = 1;
  captured_frame_.width = width_;
  captured_frame_.height = height_;

  VideoFormat format(width_, height_, VideoFormat::kMinimumInterval,
                     captured_frame_.fourcc);
  std::vector<VideoFormat> supported;
  supported.push_back(format);
  SetSupportedFormats(supported);
}

CaptureState CustomizedFramesCapturer::Start(const VideoFormat& capture_format) {
  if (IsRunning()) {
    LOG(LS_ERROR) << "Yuv Frame Generator is already running";
    return CS_FAILED;
  }
  SetCaptureFormat(&capture_format);

  worker_thread_ = rtc::Thread::Current();
  RTC_DCHECK(!async_invoker_);
  async_invoker_.reset(new rtc::AsyncInvoker());
  // Create a thread to generate frames.
  frames_generator_thread = new CustomizedFramesThread(this, fps_);
  bool ret = frames_generator_thread->Start();
  if (ret) {
    LOG(LS_INFO) << "Yuv Frame Generator started";
    return CS_RUNNING;
  } else {
    async_invoker_.reset();
    worker_thread_ = nullptr;
    LOG(LS_ERROR) << "Yuv Frame Generator failed to start";
    return CS_FAILED;
  }
}

bool CustomizedFramesCapturer::IsRunning() {
  return frames_generator_thread && !frames_generator_thread->Finished();
}

void CustomizedFramesCapturer::Stop() {
  if (frames_generator_thread) {
    frames_generator_thread->Stop();
    frames_generator_thread = NULL;
    LOG(LS_INFO) << "Yuv Frame Generator stopped";
  }
  SetCaptureFormat(NULL);
  worker_thread_ = nullptr;
  async_invoker_.reset();
}

bool CustomizedFramesCapturer::GetPreferredFourccs(std::vector<uint32>* fourccs) {
  if (!fourccs) {
    return false;
  }
  fourccs->push_back(GetSupportedFormats()->at(0).fourcc);
  return true;
}

void CustomizedFramesCapturer::sendCapturedFrame() {
  SignalFrameCaptured(this, &captured_frame_);
}

// Executed in the context of CustomizedFramesThread.
void CustomizedFramesCapturer::ReadFrame() {
  // 1. Signal the previously read frame to downstream in worker_thread.
  rtc::CritScope lock(&lock_);
  uint8 *buffer;
  frame_generator_->GenerateNextFrame(&buffer);
  captured_frame_.time_stamp = webrtc::TickTime::MillisecondTimestamp() * rtc::kNumNanosecsPerMillisec;
  frame_data_size_ = frame_generator_->GetFrameSize();
  captured_frame_.data_size = frame_data_size_;
  captured_frame_.data = new char[frame_data_size_];
  memmove(captured_frame_.data, buffer, frame_data_size_);
  delete buffer;
  async_invoker_->AsyncInvoke<void>(worker_thread_, rtc::Bind(&CustomizedFramesCapturer::sendCapturedFrame , this));
}

} // namespace base
} // namespace woogeen
