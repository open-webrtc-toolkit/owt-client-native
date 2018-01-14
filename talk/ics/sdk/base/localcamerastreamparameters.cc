/*
 * Intel License
 */

#include <random>
#include "talk/ics/sdk/include/cpp/ics/base/localcamerastreamparameters.h"

namespace ics {
namespace base {

LocalCameraStreamParameters::LocalCameraStreamParameters(bool video_enabled,
                                                         bool audio_enabled)
    : resolution_width_(320),
      resolution_height_(240),
      fps_(30),
      video_enabled_(video_enabled),
      audio_enabled_(audio_enabled) {
  std::random_device rd;
  std::string random_number = std::to_string(rd());
  stream_name_ = "ICS-Stream-" + random_number;
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
    bool video_enabled,
    bool audio_enabled,
    DesktopSourceType source_type,
    DesktopCapturePolicy capture_policy)
    : video_enabled_(video_enabled),
      audio_enabled_(audio_enabled),
      fps_(30),
      source_type_(source_type),
      capture_policy_(capture_policy) {}

void LocalDesktopStreamParameters::Fps(int fps) {
  fps_ = fps;
}
}
}
