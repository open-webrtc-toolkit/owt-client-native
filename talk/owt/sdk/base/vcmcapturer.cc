/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "talk/owt/sdk/base/vcmcapturer.h"

#include <stdint.h>
#include <memory>

#include "modules/video_capture/video_capture_factory.h"
#include "webrtc/rtc_base/bind.h"
#include "webrtc/rtc_base/checks.h"
#include "webrtc/rtc_base/logging.h"

// This file is borrowed from webrtc project.
namespace owt {
namespace base {

VcmCapturer::VcmCapturer()
    : vcm_(nullptr),
      vcm_thread_(rtc::Thread::CreateWithSocketServer()) {
  vcm_thread_->Start();
}

bool VcmCapturer::Init(size_t width,
                       size_t height,
                       size_t target_fps,
                       size_t capture_device_index) {
  std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> device_info(
      webrtc::VideoCaptureFactory::CreateDeviceInfo());

  char device_name[256];
  char unique_name[256];
  if (device_info->GetDeviceName(static_cast<uint32_t>(capture_device_index),
                                 device_name, sizeof(device_name), unique_name,
                                 sizeof(unique_name)) != 0) {
    Destroy();
    return false;
  }

  vcm_ = vcm_thread_->Invoke<rtc::scoped_refptr<webrtc::VideoCaptureModule>>(
      RTC_FROM_HERE,
      Bind(&VcmCapturer::CreateDeviceOnVCMThread, this,
                          unique_name));
  if (!vcm_) {
    return false;
  }
  vcm_->RegisterCaptureDataCallback(this);

  device_info->GetCapability(vcm_->CurrentDeviceName(), 0, capability_);

  capability_.width = static_cast<int32_t>(width);
  capability_.height = static_cast<int32_t>(height);
  capability_.maxFPS = static_cast<int32_t>(target_fps);
  capability_.videoType = webrtc::VideoType::kI420;

  if (vcm_thread_->Invoke<int32_t>(
      RTC_FROM_HERE,
      Bind(&VcmCapturer::StartCaptureOnVCMThread, this, capability_)) != 0) {
    Destroy();
    return false;
  }

  return true;
}
rtc::scoped_refptr<webrtc::VideoCaptureModule>
VcmCapturer::CreateDeviceOnVCMThread(
    const char* unique_device_utf8) {
  return webrtc::VideoCaptureFactory::Create(unique_device_utf8);
}

int32_t VcmCapturer::StartCaptureOnVCMThread(
    webrtc::VideoCaptureCapability capability) {
  return vcm_->StartCapture(capability);
}

int32_t VcmCapturer::StopCaptureOnVCMThread() {
  return vcm_->StopCapture();
}

void VcmCapturer::ReleaseOnVCMThread() {
  vcm_ = nullptr;
}

VcmCapturer* VcmCapturer::Create(size_t width,
                                 size_t height,
                                 size_t target_fps,
                                 size_t capture_device_index) {
  std::unique_ptr<VcmCapturer> vcm_capturer(new VcmCapturer());
  if (!vcm_capturer->Init(width, height, target_fps, capture_device_index)) {
    RTC_LOG(LS_WARNING) << "Failed to create VcmCapturer(w = " << width
                        << ", h = " << height << ", fps = " << target_fps
                        << ")";
    return nullptr;
  }
  return vcm_capturer.release();
}

void VcmCapturer::Destroy() {
  if (!vcm_)
    return;

  vcm_thread_->Invoke<int32_t>(RTC_FROM_HERE,
                            Bind(&VcmCapturer::StopCaptureOnVCMThread, this));
  vcm_->DeRegisterCaptureDataCallback();
  // Release reference to VCM.
  vcm_thread_->Invoke<void>(RTC_FROM_HERE,
                            Bind(&VcmCapturer::ReleaseOnVCMThread, this));
}

VcmCapturer::~VcmCapturer() {
  Destroy();
  vcm_thread_->Stop();
}

void VcmCapturer::OnFrame(const webrtc::VideoFrame& frame) {
  CameraVideoCapturer::OnFrame(frame);
}

}  // namespace base
}  // namespace owt
