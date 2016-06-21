/*
 * Intel License
 */

#include "talk/woogeen/sdk/include/cpp/woogeen/base/deviceutils.h"
#include "webrtc/base/arraysize.h"
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
    // Try to get capabilities by device name if getting capabilities by ID is
    // failed.
    // TODO(jianjun): Remove this when creating stream by device name is no
    // longer supported.
    if (resolutions.size() == 0) {
      // Get device ID by name.
      int num_cams = info->NumberOfDevices();
      char vcm_id[256] = "";
      bool found = false;
      for (int index = 0; index < num_cams; ++index) {
        char vcm_name[256] = "";
        if (info->GetDeviceName(index, vcm_name, arraysize(vcm_name), vcm_id,
                                arraysize(vcm_id)) != -1) {
          if (id == reinterpret_cast<char*>(vcm_name)) {
            found = true;
            break;
          }
        }
      }
      if (found) {
        for (int32_t i = 0; i < info->NumberOfCapabilities(vcm_id); i++) {
          if (info->GetCapability(vcm_id, i, capability) == 0) {
            resolutions.push_back(
                Resolution(capability.width, capability.height));
          } else {
            LOG(LS_WARNING) << "Failed to get capability.";
          }
        }
      }
    }
  }
  return resolutions;
}

}
}
