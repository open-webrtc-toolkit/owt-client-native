/*
 * Intel License
 */

#ifndef WOOGEEN_BASE_AVFOUNDATIONVIDEOCAPTURERFACTORY_H_
#define WOOGEEN_BASE_AVFOUNDATIONVIDEOCAPTURERFACTORY_H_

#include "talk/woogeen/sdk/include/cpp/woogeen/base/localcamerastreamparameters.h"
#include "webrtc/media/base/videocapturer.h"

namespace woogeen {
namespace base {
class AVFoundationVideoCapturerFactory {
 public:
  static cricket::VideoCapturer* Create(
      const LocalCameraStreamParameters& parameters);
};
}
}

#endif  // WOOGEEN_BASE_AVFOUNDATIONVIDEOCAPTURERFACTORY_H_
