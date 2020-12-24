// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#if defined(WEBRTC_LINUX)
#include <sys/utsname.h>
#endif  // WEBRTC_LINUX
#include "webrtc/rtc_base/checks.h"
#include "talk/owt/sdk/base/sysinfo.h"
namespace owt {
namespace base {
static const std::string kSdkVersion("5.0");
static const std::string kRuntimeName("WebRTC");
static const std::string kRuntimeVersion("83");
static const std::string kUnknown("Unknown");
std::string SysInfo::SdkType() {
#if defined(WEBRTC_IOS) || defined(WEBRTC_MAC)
  return "Objective-C";
#else
  return "C++";
#endif
}
#if defined(WEBRTC_WIN)
std::string SysInfo::OsName() {
  return "Windows NT";
}
std::string SysInfo::OsVersion() {
  // TODO: Implement it.
  return kUnknown;
}
#endif  // WEBRTC_WIN
#if defined(WEBRTC_LINUX)
std::string SysInfo::OsName(){
  struct utsname info;
  if (uname(&info) < 0) {
    RTC_NOTREACHED();
    return kUnknown;
  }
  return std::string(info.sysname);
}
std::string SysInfo::OsVersion() {
  struct utsname info;
  if (uname(&info) < 0) {
    RTC_NOTREACHED();
    return kUnknown;
  }
  return std::string(info.release);
}
#endif  // WEBRTC_LINUX
SysInfo SysInfo::GetInstance() {
  SdkInfo sdk(SdkType(), kSdkVersion);
  OsInfo os(OsName(), OsVersion());
  RuntimeInfo runtime(kRuntimeName, kRuntimeVersion);
  return SysInfo(sdk, os, runtime);
}
SysInfo::SysInfo(const SdkInfo sdk_info,
                 const OsInfo os_info,
                 const RuntimeInfo runtime_info)
    : sdk(sdk_info), os(os_info), runtime(runtime_info) {}
}
}
