// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_SYSINFO_H_
#define OWT_BASE_SYSINFO_H_
#include <string>
namespace owt {
namespace base {
/// OWT SDK info.
struct SdkInfo {
  SdkInfo(const std::string& type, const std::string& version)
      : type(type), version(version) {}
  const std::string type;
  const std::string version;
};
/// Operating system info.
struct OsInfo {
  OsInfo(const std::string& name, const std::string& version)
      : name(name), version(version) {}
  const std::string name;
  const std::string version;
};
/// WebRTC runtime info.
struct RuntimeInfo {
  RuntimeInfo(const std::string& name, const std::string& version)
      : name(name), version(version) {}
  const std::string name;
  const std::string version;
};
/// System information.
class SysInfo final{
 public:
  /// Get system info.
  static SysInfo GetInstance();
  const SdkInfo sdk;
  const OsInfo os;
  const RuntimeInfo runtime;
 private:
  SysInfo(const SdkInfo sdk_info,
          const OsInfo os_info,
          const RuntimeInfo runtime_info);
  static std::string SdkType();
  static std::string OsName();
  static std::string OsVersion();
};
}
}
#endif  // WOOGEEN_BASE_SYSINFO_H_
