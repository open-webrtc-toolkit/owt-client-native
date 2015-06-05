/*
 * Intel License
 */

#ifndef WOOGEEN_BASE_LOCALCAMERASTREAMPARAMETERS_H_
#define WOOGEEN_BASE_LOCALCAMERASTREAMPARAMETERS_H_

namespace woogeen {
class LocalCameraStreamParameters final {
  public:
    LocalCameraStreamParameters(bool has_video, bool has_audio);

  private:
    int resolution_width;
    int resolution_height;
    int fps;

}
}

#endif  // WOOGEEN_BASE_LOCALCAMERASTREAMPARAMETERS_H_
