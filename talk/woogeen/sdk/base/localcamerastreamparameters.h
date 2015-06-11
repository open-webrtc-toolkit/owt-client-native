/*
 * Intel License
 */

#ifndef WOOGEEN_BASE_LOCALCAMERASTREAMPARAMETERS_H_
#define WOOGEEN_BASE_LOCALCAMERASTREAMPARAMETERS_H_

#include <string>

namespace woogeen {
class LocalCameraStreamParameters final {
  friend class LocalCameraStream;

  public:
    LocalCameraStreamParameters(bool video_enabled, bool audio_enabled);
    void CameraId(std::string& camera_id);
    void StreamName(std::string& stream_name);
    void Resolution(int width, int height);
    void Fps(int fps);

  private:
    std::shared_ptr<std::string> camera_id_;
    std::shared_ptr<std::string> stream_name_;
    int resolution_width_;
    int resolution_height_;
    int fps_;
    bool video_enabled_;
    bool audio_enabled_;

};
}

#endif  // WOOGEEN_BASE_LOCALCAMERASTREAMPARAMETERS_H_
