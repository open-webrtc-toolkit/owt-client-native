/*
 * Intel License
 */

#ifndef ICS_BASE_OBJC_CAMERAVIDEOCAPTUREFACTORY_H_
#define ICS_BASE_OBJC_CAMERAVIDEOCAPTUREFACTORY_H_

#include <memory>
#include "talk/ics/sdk/base/objc/ObjcVideoCapturerInterface.h"
#include "talk/ics/sdk/include/cpp/ics/base/localcamerastreamparameters.h"
#include "third_party/webrtc/api/mediastreaminterface.h"
#include "third_party/webrtc/rtc_base/scoped_ref_ptr.h"

@class RTCVideoCapturer;
@class RTCVideoSource;

namespace ics {
namespace base {
/// Wraps an RTCVideoCapturer.
class CameraVideoCapturer : public ObjcVideoCapturerInterface {
 public:
  rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> source() override;
  CameraVideoCapturer(RTCVideoCapturer* capturer, RTCVideoSource* source);
  ~CameraVideoCapturer() override;

 private:
  RTCVideoCapturer* capturer_;
  RTCVideoSource* source_;
};
}  // namespace base
}  // namespace woogeen

#endif  // ICS_BASE_OBJC_CAMERAVIDEOCAPTUREFACTORY_H_
