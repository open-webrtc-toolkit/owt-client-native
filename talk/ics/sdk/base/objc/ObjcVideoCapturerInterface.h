/*
 * Intel License
 */

#ifndef WOOGEEN_BASE_OBJC_OBJCVIDEOCAPTUREINTERFACE_H_
#define WOOGEEN_BASE_OBJC_OBJCVIDEOCAPTUREINTERFACE_H_

#include "talk/ics/sdk/include/cpp/ics/base/localcamerastreamparameters.h"
#include "third_party/webrtc/rtc_base/scoped_ref_ptr.h"
#include "third_party/webrtc/api/mediastreaminterface.h"

namespace ics {
namespace base {
/**
  @brief Video capturer interface for ObjC implementation.
  @details It's too complex to implement a cricket::VideoCapturer for ObjC video
  capturers. However, ObjcVideoTrackSource does not hold an reference to
  capturer. So we need to hold a reference in C++ code to avoid capturer being
  destroyed.
  */
class ObjcVideoCapturerInterface {
 public:
  virtual rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> source() = 0;
  virtual ~ObjcVideoCapturerInterface(){};
};

class ObjcVideoCapturerFactory {
 public:
  static std::unique_ptr<ObjcVideoCapturerInterface> Create(
      const LocalCameraStreamParameters& parameters);
};
}  // namespace base
}  // namespace woogeen

#endif  // WOOGEEN_BASE_OBJC_OBJCVIDEOCAPTUREINTERFACE_H_
