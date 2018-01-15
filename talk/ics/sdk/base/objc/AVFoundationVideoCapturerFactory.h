/*
 * Intel License
 */

#ifndef WOOGEEN_BASE_AVFOUNDATIONVIDEOCAPTURERFACTORY_H_
#define WOOGEEN_BASE_AVFOUNDATIONVIDEOCAPTURERFACTORY_H_

#include "talk/ics/sdk/include/cpp/ics/base/localcamerastreamparameters.h"
#include "webrtc/media/base/videocapturer.h"

namespace ics {
namespace base {
class AVFoundationVideoCapturerFactory {
 public:
  static std::unique_ptr<cricket::VideoCapturer> Create(
      const LocalCameraStreamParameters& parameters);
};
}
}

#endif  // WOOGEEN_BASE_AVFOUNDATIONVIDEOCAPTURERFACTORY_H_
