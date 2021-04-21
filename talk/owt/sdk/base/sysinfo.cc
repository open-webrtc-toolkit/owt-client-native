// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#if defined(WEBRTC_LINUX)
#include <sys/utsname.h>
#endif  // WEBRTC_LINUX
#include "webrtc/rtc_base/checks.h"
#if defined(WEBRTC_WIN)
#include <memory>
#include "webrtc/rtc_base/win/windows_version.h"
#endif
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
using namespace rtc::rtc_win;
std::string SysInfo::OsName() {
  Version version = GetVersion();
  switch (version) {
    case VERSION_PRE_XP:
      return "Pre-Windows XP";
    case VERSION_XP:
      return "Windows XP";
    case VERSION_SERVER_2003:
      return "Windows Server 2003";
    case VERSION_VISTA:
      return "Windows Vista";
    case VERSION_WIN7:
      return "Windows 7";
    case VERSION_WIN8:
      return "Windows 8";
    case VERSION_WIN8_1:
      return "Windows 8.1";  // May also be Windows Server 2012 R1.
    // Be noted if the application is not manifest for Win8.1 or Win10, it will aways
    // Win8 for version higher than that.
    case VERSION_WIN10:      // TH 10586
    case VERSION_WIN10_TH2:  // TH 14393
      return "Windows 10 Threshold";
    case VERSION_WIN10_RS1:
      return "Windows 10 RS1";  // Redstone 1
    case VERSION_WIN10_RS2:
      return "Windows 10 RS2";  // Redstone 2
    case VERSION_WIN10_RS3:
      return "Windows 10 RS3";  // Redstone 3, Build 16299
    case VERSION_WIN10_RS4:
      return "Windows 10 RS4";  // Redstone 4, Version 1803.
    default:
      return kUnknown;
  }
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
