// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_WIN_DEVICE_INFO_MF_H
#define OWT_BASE_WIN_DEVICE_INFO_MF_H

#include <mfapi.h>
#include <mferror.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <strsafe.h>
#include <windows.h>
#include "webrtc/modules/video_capture/device_info_impl.h"
#include "webrtc/rtc_base/synchronization/mutex.h"
#include "webrtc/rtc_base/thread_annotations.h"


namespace webrtc {
namespace videocapturemodule {

#define SAFE_RELEASE(punknown) \
  if ((punknown) != nullptr) {    \
    (punknown)->Release();     \
    (punknown) = nullptr;         \
  }

class DeviceInfoMF : public DeviceInfoImpl {
public:
  static DeviceInfoMF* Create();

  DeviceInfoMF();
  ~DeviceInfoMF() override;

  int32_t Init() override;
  uint32_t NumberOfDevices() override;

  /*
   * Returns the available capture devices.
   */
  int32_t GetDeviceName(uint32_t deviceNumber,
                        char* deviceNameUTF8,
                        uint32_t deviceNameLength,
                        char* deviceUniqueIdUTF8,
                        uint32_t deviceUniqueIdUTF8Length,
                        char* productUniqueIdUTF8,
                        uint32_t productUniqueIdUTF8Length) override;
  /*
   * Display OS /capture device specific settings dialog
   */
  int32_t DisplayCaptureSettingsDialogBox(const char* deviceUniqueIdUTF8,
                                          const char* dialogTitleUTF8,
                                          void* parentWindow,
                                          uint32_t positionX,
                                          uint32_t positionY) override;
  /*
   * Creates a Capture Device Media Source
   * Responsible by the user to release the MediaSource
   */
  IMFMediaSource* GetCaptureSource(const char*& deviceUniqueIdUTF8);

protected:
  int32_t GetDeviceInfo(uint32_t deviceNumber,
                        char* deviceNameUTF8,
                        uint32_t deviceNameLength,
                        char* deviceUniqueIdUTF8,
                        uint32_t deviceUniqueIdUTF8Length,
                        char* productUniqueIdUTF8,
                        uint32_t productUniqueIdUTF8Length);

  int32_t CreateCapabilityMap(const char* deviceUniqueIdUTF8) override
      RTC_EXCLUSIVE_LOCKS_REQUIRED(_apiLock);
  ;
};

}  // namespace videocapturemodule
}  // namespace webrtc
#endif  // OWT_BASE_WIN_DEVICE_INFO_MF_H
