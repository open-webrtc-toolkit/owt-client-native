/*
 * Intel License
 */

#ifndef WOOGEEN_BASE_LOCALCAMERASTREAMPARAMETERS_H_
#define WOOGEEN_BASE_LOCALCAMERASTREAMPARAMETERS_H_

#include <string>

namespace woogeen {
class LocalCameraStreamParameters final {

  public:
    LocalCameraStreamParameters(bool video_enabled, bool audio_enabled);
    void CameraId(std::string& camera_id);
    void StreamName(std::string& stream_name);
    void Resolution(int width, int height);
    void Fps(int fps);
    std::string CameraId() {return camera_id_;}
    std::string StreamName() {return stream_name_;}
    int ResolutionWidth() {return resolution_width_;}
    int ResolutionHeight() {return resolution_height_;}
    int Fps() {return fps_;}
    bool VideoEnabled() {return video_enabled_;}
    bool AudioEnabled() {return audio_enabled_;}

  private:
    std::string camera_id_;
    std::string stream_name_;
    int resolution_width_;
    int resolution_height_;
    int fps_;
    bool video_enabled_;
    bool audio_enabled_;

};
}

#endif  // WOOGEEN_BASE_LOCALCAMERASTREAMPARAMETERS_H_
