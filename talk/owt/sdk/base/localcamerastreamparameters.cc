// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include <random>
#include "talk/owt/sdk/include/cpp/owt/base/localcamerastreamparameters.h"
namespace owt {
namespace base {
LocalCameraStreamParameters::LocalCameraStreamParameters(bool audio_enabled,
                                                         bool video_enabled)
    : resolution_width_(320),
      resolution_height_(240),
      fps_(30),
      video_enabled_(video_enabled),
      audio_enabled_(audio_enabled) {
  std::random_device rd;
  std::string random_number = std::to_string(rd());
  stream_name_ = "OWT-Stream-" + random_number;
}
void LocalCameraStreamParameters::Fps(int fps) {
  fps_ = fps;
}
void LocalCameraStreamParameters::CameraId(const std::string& camera_id) {
  camera_id_ = camera_id;
}
void LocalCameraStreamParameters::Resolution(int width, int height) {
  resolution_width_ = width;
  resolution_height_ = height;
}
void LocalCameraStreamParameters::StreamName(const std::string& stream_name){
  stream_name_ = stream_name;
}
LocalDesktopStreamParameters::LocalDesktopStreamParameters(
    bool audio_enabled,
    bool video_enabled)
    : video_enabled_(video_enabled),
      audio_enabled_(audio_enabled),
      fps_(30),
      source_type_(DesktopSourceType::kFullScreen),
      capture_policy_(DesktopCapturePolicy::kDefault) {}
void LocalDesktopStreamParameters::Fps(int fps) {
  fps_ = fps;
}
}
}
