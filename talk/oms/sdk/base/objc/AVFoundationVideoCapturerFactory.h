/*
 * Intel License
 */

#ifndef WOOGEEN_BASE_AVFOUNDATIONVIDEOCAPTURERFACTORY_H_
#define WOOGEEN_BASE_AVFOUNDATIONVIDEOCAPTURERFACTORY_H_

#include "talk/oms/sdk/include/cpp/oms/base/localcamerastreamparameters.h"
#include "webrtc/media/base/videocapturer.h"

namespace oms {
namespace base {
class AVFoundationVideoCapturerFactory {
 public:
  static std::unique_ptr<cricket::VideoCapturer> Create(
      const LocalCameraStreamParameters& parameters);
};
}
}

#endif  // WOOGEEN_BASE_AVFOUNDATIONVIDEOCAPTURERFACTORY_H_
