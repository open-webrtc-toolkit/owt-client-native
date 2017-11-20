/*
 * Intel License
 */

#include "talk/woogeen/sdk/base/objc/CameraVideoCapturer.h"
#include "talk/woogeen/sdk/include/objc/Woogeen/RTCFactory.h"
#include "third_party/webrtc/rtc_base/logging.h"
#include "third_party/webrtc/sdk/objc/Framework/Classes/PeerConnection/RTCVideoSource+Private.h"
#include "third_party/webrtc/sdk/objc/Framework/Headers/WebRTC/RTCCameraVideoCapturer.h"

namespace woogeen {
namespace base {

std::unique_ptr<ObjcVideoCapturerInterface> ObjcVideoCapturerFactory::Create(
    const LocalCameraStreamParameters& parameters) {
  RTCVideoSource* source = [RTCFactory videoSource];
  RTCCameraVideoCapturer* capturer =
      [[RTCCameraVideoCapturer alloc] initWithDelegate:source];
  AVCaptureDevicePosition position = AVCaptureDevicePositionFront;
  if (parameters.CameraId() ==
          "com.apple.avfoundation.avcapturedevice.built-in_video:0" ||
      parameters.CameraId() == "Back Camera") {
    position = AVCaptureDevicePositionBack;
  } else if (parameters.CameraId() ==
                 "com.apple.avfoundation.avcapturedevice.built-in_video:1" ||
             parameters.CameraId() == "Front Camera") {
    position = AVCaptureDevicePositionFront;
  } else {
    LOG(LS_ERROR) << "Cannot find suitable camera device.";
    RTC_NOTREACHED();
    return nullptr;
  }
  NSArray<AVCaptureDevice*>* capture_devices =
      [RTCCameraVideoCapturer captureDevices];
  AVCaptureDevice* device = nullptr;
  for (AVCaptureDevice* d in capture_devices) {
    if (d.position == position) {
      device = d;
      break;
    }
  }
  if (device == nullptr) {
    LOG(LS_ERROR) << "Cannot find suitable camera devices.";
    return nullptr;
  }
  AVCaptureDeviceFormat* format = nil;
  NSArray<AVCaptureDeviceFormat*>* formats =
      [RTCCameraVideoCapturer supportedFormatsForDevice:device];
  for (AVCaptureDeviceFormat* f in formats) {
    CMVideoDimensions dimension =
        CMVideoFormatDescriptionGetDimensions(f.formatDescription);
    if (dimension.width == parameters.ResolutionWidth() &&
        dimension.height == parameters.ResolutionHeight()) {
      format = f;
      break;
    }
  }
  if (format == nil) {
    LOG(LS_ERROR) << "Cannot open camera with suitable format.";
    return nullptr;
  }
  bool valid_fps(false);
  for (AVFrameRateRange* fps_range in format.videoSupportedFrameRateRanges) {
    if (parameters.Fps() >= fps_range.minFrameRate &&
        parameters.Fps() <= fps_range.maxFrameRate) {
      valid_fps = true;
      break;
    }
  }
  if (!valid_fps) {
    LOG(LS_ERROR) << "Cannot open camera with suitable FPS.";
    return nullptr;
  }
  [capturer startCaptureWithDevice:device format:format fps:parameters.Fps()];
  return std::unique_ptr<CameraVideoCapturer>(
      new CameraVideoCapturer(capturer, source));
}

CameraVideoCapturer::CameraVideoCapturer(RTCVideoCapturer* capturer,
                                         RTCVideoSource* source)
    : capturer_(capturer), source_(source) {
  RTC_DCHECK(capturer_);
  RTC_DCHECK(source_);
}

rtc::scoped_refptr<webrtc::VideoTrackSourceInterface>
CameraVideoCapturer::source() {
  return [source_ nativeVideoSource];
}

CameraVideoCapturer::~CameraVideoCapturer() {
  // Remove it once a capturer can be shared among multiple streams.
  if ([capturer_ isKindOfClass:[RTCCameraVideoCapturer class]]) {
    RTCCameraVideoCapturer *cameraCapturer =
        static_cast<RTCCameraVideoCapturer *>(capturer_);
    [cameraCapturer stopCapture];
  }
  capturer_ = nil;
  source_ = nil;
}

}
}
