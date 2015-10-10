/*
 * Intel License
 */

#include <random>
#include "talk/woogeen/sdk/base/localcamerastreamparameters.h"

namespace woogeen {

LocalCameraStreamParameters::LocalCameraStreamParameters(bool video_enabled,
                                                         bool audio_enabled)
    : video_enabled_(video_enabled),
      audio_enabled_(audio_enabled),
      resolution_width_(320),
      resolution_height_(240),
      fps_(30) {
  std::random_device rd;
  std::string random_number = std::to_string(rd());
  stream_name_ = "WooGeen-Stream-" + random_number;
}

void LocalCameraStreamParameters::Fps(int fps) {
  fps_ = fps;
}

void LocalCameraStreamParameters::CameraId(std::string& camera_id) {
  camera_id_ = camera_id;
}

void LocalCameraStreamParameters::Resolution(int width, int height) {
  resolution_width_ = width;
  resolution_height_ = height;
}
}
