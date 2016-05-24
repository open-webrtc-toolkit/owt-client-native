/*
 * Intel License
 */

#include "talk/woogeen/sdk/include/cpp/woogeen/base/deviceutils.h"
#include "webrtc/base/logging.h"
#include "webrtc/media/base/videocapturer.h"
#include "webrtc/modules/video_capture/video_capture_factory.h"
#include "webrtc/media/engine/webrtcvideocapturerfactory.h"

namespace woogeen {
namespace base {
std::vector<std::string> DeviceUtils::VideoCapturerIds() {
  std::vector<std::string> device_ids;
  std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> info(
      webrtc::VideoCaptureFactory::CreateDeviceInfo(0));
  if (!info) {
    LOG(LS_ERROR) << "CreateDeviceInfo failed";
  } else {
    int num_devices = info->NumberOfDevices();
    for (int i = 0; i < num_devices; ++i) {
      const uint32_t kSize = 256;
      char name[kSize] = {0};
      char id[kSize] = {0};
      if (info->GetDeviceName(i, name, kSize, id, kSize) != -1) {
        device_ids.push_back(id);
      }
    }
  }
  return device_ids;
}

std::vector<Resolution> DeviceUtils::VideoCapturerSupportedResolutions(
    const std::string& id) {
  std::vector<Resolution> resolutions;
  webrtc::VideoCaptureCapability capability;
  std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> info(
      webrtc::VideoCaptureFactory::CreateDeviceInfo(0));
  if (!info) {
    LOG(LS_ERROR) << "CreateDeviceInfo failed";
  } else {
    for (int32_t i = 0; i < info->NumberOfCapabilities(id.c_str()); i++) {
      if (info->GetCapability(id.c_str(), i, capability) == 0) {
        resolutions.push_back(Resolution(capability.width, capability.height));
      } else {
        LOG(LS_WARNING) << "Failed to get capability.";
      }
    }
  }
  return resolutions;
}

}
}
