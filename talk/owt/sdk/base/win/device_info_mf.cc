// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/base/win/device_info_mf.h"

#include <atlbase.h>
#include <atlcom.h>
#include <system_error>
#include "absl/strings/match.h"
#include "webrtc/api/scoped_refptr.h"
#include "webrtc/rtc_base/logging.h"
#include "webrtc/rtc_base/string_utils.h"

#pragma comment(lib, "Mfuuid.lib")
#pragma comment(lib, "Mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "Mf.lib")

namespace webrtc {
namespace videocapturemodule {

DeviceInfoMF* DeviceInfoMF::Create() {
  DeviceInfoMF* ds_info = new (std::nothrow) DeviceInfoMF();
  if (!ds_info || ds_info->Init() != 0) {
    delete ds_info;
    ds_info = nullptr;
  }
  return ds_info;
}

DeviceInfoMF::DeviceInfoMF() {
  // Maybe we should move this to Init().
  HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  if (FAILED(hr)) {
    if (hr == RPC_E_CHANGED_MODE) {
      RTC_LOG(LS_WARNING)
          << "CoInitializeEx failed:" << std::system_category().message(hr);
    }
    return;
  }
}

DeviceInfoMF::~DeviceInfoMF() {
  MFShutdown();
  CoUninitialize();
}

int32_t DeviceInfoMF::Init() {
  HRESULT hr = MFStartup(MF_VERSION);
  if (FAILED(hr)) {
    RTC_LOG(LS_WARNING) << "MFStartup failed:"
                        << std::system_category().message(hr);
    return -1;
  }
  return 0;
}

uint32_t DeviceInfoMF::NumberOfDevices() {
  webrtc::MutexLock lock(&_apiLock);
  return GetDeviceInfo(0, 0, 0, 0, 0, 0, 0);
}

int32_t DeviceInfoMF::GetDeviceName(uint32_t deviceNumber,
                                  char* deviceNameUTF8,
                                  uint32_t deviceNameLength,
                                  char* deviceUniqueIdUTF8,
                                  uint32_t deviceUniqueIdUTF8Length,
                                  char* productUniqueIdUTF8,
                                  uint32_t productUniqueIdUTF8Length) {
  webrtc::MutexLock lock(&_apiLock);
  const int32_t result =
      GetDeviceInfo(deviceNumber, deviceNameUTF8, deviceNameLength,
                    deviceUniqueIdUTF8, deviceUniqueIdUTF8Length,
                    productUniqueIdUTF8, productUniqueIdUTF8Length);
  return (result > (int32_t)deviceNumber) ? 0 : -1;
}

int32_t DeviceInfoMF::GetDeviceInfo(uint32_t deviceNumber,
                                    char* deviceNameUTF8,
                                    uint32_t deviceNameLength,
                                    char* deviceUniqueIdUTF8,
                                    uint32_t deviceUniqueIdUTF8Length,
                                    char* productUniqueIdUTF8,
                                    uint32_t productUniqueIdUTF8Length) {
  CComPtr<IMFAttributes> attributes = nullptr;
  CComHeapPtr<IMFActivate*> devices;
  int32_t num_devices = -1;
  uint32_t source_count = 0;

  HRESULT hr = MFCreateAttributes(&attributes, 1);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "MFCreateAttributes failed:"
                      << std::system_category().message(hr);
    return num_devices;
  }

  // Request the video capture devices by setting GUID
  hr = attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
                            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "SetGUID failed:"
                      << std::system_category().message(hr);
    return num_devices;
  }

  // Enumerate the devices
  hr = MFEnumDeviceSources(attributes, &devices, &source_count);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "MFEnumDeviceSources failed:"
                      << std::system_category().message(hr);
    return num_devices;
  }

  for (uint32_t idx = 0; idx < source_count; idx++) {
    if (deviceNumber == idx) {
      // Found a valid device
      int ret = 0;
      CComHeapPtr<WCHAR> device_name;
      hr = devices[idx]->GetAllocatedString(
          MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &device_name, nullptr);
      if (FAILED(hr)) {
        RTC_LOG(LS_ERROR) << "Getting device friendly name failed:"
                          << std::system_category().message(hr);
      }
      else if (deviceNameLength > 0) {
        ret = WideCharToMultiByte(CP_UTF8, 0, device_name, -1,
                                         (char*)deviceNameUTF8,
                                         deviceNameLength, nullptr, nullptr);
        if (!ret) {
          RTC_LOG(LS_ERROR) << "Failed to convert device name to UTF8, error = "
              << GetLastError();
          break;
        }
      }

      CComHeapPtr<WCHAR> device_unique_name;
      hr = devices[idx]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK,
          &device_unique_name, nullptr);
      if (FAILED(hr)) {
        RTC_LOG(LS_ERROR) << "Getting device friendly name failed:"
                          << std::system_category().message(hr);
      }
      else if (deviceUniqueIdUTF8Length > 0) {
        ret = WideCharToMultiByte(CP_UTF8, 0, device_unique_name, -1,
                                         (char*)deviceUniqueIdUTF8,
                                         deviceUniqueIdUTF8Length, nullptr, nullptr);
        if (!ret) {
          RTC_LOG(LS_ERROR) << "Failed to convert device name to UTF8, error = "
              << GetLastError();
          break;
        }
      }
      num_devices = source_count;
      break;
    }
  }

  for (uint32_t i = 0; i < source_count; i++) {
    SAFE_RELEASE(devices[i]);
  }

  return num_devices;
}

int32_t DeviceInfoMF::DisplayCaptureSettingsDialogBox(
    const char* deviceUniqueIdUTF8,
    const char* dialogTitleUTF8,
    void* parentWindow,
    uint32_t positionX,
    uint32_t positionY) {
  return -1;
}

IMFMediaSource* DeviceInfoMF::GetCaptureSource(const char*& deviceUniqueIdUTF8) {
  CComPtr<IMFAttributes> attributes = nullptr;
  IMFMediaSource* source = nullptr;
  std::wstring deviceUniqueName(deviceUniqueIdUTF8,
      deviceUniqueIdUTF8 + strlen(deviceUniqueIdUTF8));

  // Max Count will be kVideoCaptureUniqueNameLength(Max UniqueName length)
  if (deviceUniqueName.length() > kVideoCaptureUniqueNameLength) {
    RTC_LOG(LS_INFO) << "Device name too long";
    return nullptr;
  }

  HRESULT hr = MFCreateAttributes(&attributes, 1);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "MFCreateAttributes failed:"
                      << std::system_category().message(hr);
    return nullptr;
  }

  // Request the vidoe capture device by setting GUID
  hr = attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
                            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "SetGUID failed:"
                      << std::system_category().message(hr);
    return nullptr;
  }

  hr = attributes->SetString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK,
      deviceUniqueName.c_str());
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "SetUniqueName failed:"
                      << std::system_category().message(hr);
    return nullptr;
  }

  hr = MFCreateDeviceSource(attributes, &source);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "MFCreateDeviceSource failed:"
                      << std::system_category().message(hr);
  }

  return source;
}

int32_t DeviceInfoMF::CreateCapabilityMap(const char* deviceUniqueIdUTF8) {
  // Reset old capability list
  _captureCapabilities.clear();

  const int32_t deviceUniqueIdUTF8Length =
      (int32_t)strnlen_s((char*)deviceUniqueIdUTF8, kVideoCaptureUniqueNameLength);
  if (deviceUniqueIdUTF8Length > kVideoCaptureUniqueNameLength) {
    RTC_LOG(LS_INFO) << "Device name too long";
    return -1;
  }

  CComPtr<IMFMediaSource> source(GetCaptureSource(deviceUniqueIdUTF8));
  if (source == nullptr) {
    RTC_LOG(LS_ERROR) << "Unable to create capture source.";
    return -1;
  }

  CComPtr<IMFPresentationDescriptor> pd = nullptr;
  CComPtr<IMFStreamDescriptor> sd = nullptr;
  CComPtr<IMFMediaTypeHandler> mh = nullptr;
  IMFMediaType* media_type = nullptr;
  DWORD num_formats = 0;

  // Get the presentation descriptor
  HRESULT hr = source->CreatePresentationDescriptor(&pd);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "CreatePresentationDescriptor failed:"
                      << std::system_category().message(hr);
    return -1;
  }

  // Get the stream descriptor
  BOOL selected = false;
  hr = pd->GetStreamDescriptorByIndex(0/*video*/, &selected, &sd);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "GetVideoStreamDescriptor failed:"
                      << std::system_category().message(hr);
    return -1;
  }

  hr = sd->GetMediaTypeHandler(&mh);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "GetMediaTypeHandler failed:"
                      << std::system_category().message(hr);
    return -1;
  }

  hr = mh->GetMediaTypeCount(&num_formats);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "GetMediaTypeCount failed:"
                      << std::system_category().message(hr);
    return -1;
  }

  RTC_LOG(LS_INFO) << "Total number of formats supported are " << num_formats;
  for (DWORD idx = 0; idx < num_formats; idx++) {
    // Releasing on entering the loop is because if in case there was any error
    // inside the loop, helps to release the media_type for the next one.
    SAFE_RELEASE(media_type);
    // Get MediaType at Index
    hr = mh->GetMediaTypeByIndex(idx, &media_type);
    if (FAILED(hr)) {
      RTC_LOG(LS_ERROR) << "GetMediaType at index " << idx
                        << " failed:" << std::system_category().message(hr);
      continue;
    }
    VideoCaptureCapability capability;
    capability.interlaced = false;

    // Get the frame width and height
    hr = MFGetAttributeSize(media_type, MF_MT_FRAME_SIZE,
                            (UINT32*)&capability.width,
                            (UINT32*)&capability.height);
    if (FAILED(hr)) {
      RTC_LOG(LS_ERROR) << "GetFrameSize failed:"
                        << std::system_category().message(hr);
      continue;
    }

    UINT32 frame_rate = 0, denominator = 0;
    // Get the frame rate that is supported
    hr = MFGetAttributeRatio(media_type, MF_MT_FRAME_RATE, &frame_rate,
                             &denominator);
    if (FAILED(hr)) {
      RTC_LOG(LS_ERROR) << "VideoCatpurer: GetFrameRate failed:"
                        << std::system_category().message(hr);
      continue;
    }
    frame_rate = (denominator != 0) ? frame_rate / denominator : frame_rate;
    capability.maxFPS = frame_rate;

    GUID sub_type = { 0 };
    // Get the video subtype for this media.
    hr = media_type->GetGUID(MF_MT_SUBTYPE, &sub_type);
    if (FAILED(hr)) {
      RTC_LOG(LS_ERROR) << "GetMediaSubTypeGUID failed:"
                        << std::system_category().message(hr);
      continue;
    }

    // Cannot switch media type.
    if (sub_type == MFVideoFormat_I420) {
      capability.videoType = VideoType::kI420;
    } else if (sub_type == MFVideoFormat_IYUV) {
      capability.videoType = VideoType::kIYUV;
    } else if (sub_type == MFVideoFormat_RGB24) {
      capability.videoType = VideoType::kRGB24;
    } else if (sub_type == MFVideoFormat_YUY2) {
      capability.videoType = VideoType::kYUY2;
    } else if (sub_type == MFVideoFormat_NV12) {
      capability.videoType = VideoType::kNV12;
    } else if (sub_type == MFVideoFormat_RGB565) {
      capability.videoType = VideoType::kRGB565;
    } else if (sub_type == MFVideoFormat_MJPG) {
      capability.videoType = VideoType::kMJPEG;
    } else if (sub_type == MFVideoFormat_UYVY) {
      capability.videoType = VideoType::kUYVY;
    } else {
      WCHAR str_guid[32];
      StringFromGUID2(sub_type, str_guid, 32);
      RTC_LOG(LS_WARNING) << "Device support unknown media type " << str_guid
                          << ", width " << capability.width << ", height "
                          << capability.height;
      continue;
    }
    _captureCapabilities.push_back(capability);
    RTC_LOG(LS_INFO) << "Camera capability, width:" << capability.width
                     << " height:" << capability.height
                     << " type:" << static_cast<int>(capability.videoType)
                     << " fps:" << capability.maxFPS;
  }

  if (SUCCEEDED(hr)) {
    // Store the new used device name
    _lastUsedDeviceNameLength = deviceUniqueIdUTF8Length;
    _lastUsedDeviceName = (char*)realloc(_lastUsedDeviceName,
        _lastUsedDeviceNameLength + 1);
    if (_lastUsedDeviceName)
      memcpy_s(_lastUsedDeviceName, _lastUsedDeviceNameLength + 1,
          deviceUniqueIdUTF8, _lastUsedDeviceNameLength + 1);
  }
  // In case of any error in the loop
  SAFE_RELEASE(media_type);

  return SUCCEEDED(hr) ? 0 : -1;
}
} // namespace videocapturemodule
} // namespace webrtc
