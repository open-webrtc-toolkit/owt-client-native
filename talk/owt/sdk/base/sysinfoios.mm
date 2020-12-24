// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "talk/owt/sdk/base/sysinfo.h"
#import <UIKit/UIKit.h>
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
namespace owt {
namespace base {
#if defined(WEBRTC_IOS)
std::string SysInfo::OsName() {
  return "iPhone OS";
}
std::string SysInfo::OsVersion() {
  static dispatch_once_t get_system_version_once;
  static std::string* system_version;
  dispatch_once(&get_system_version_once, ^{
    system_version = new std::string(
        [NSString stdStringForString:[[UIDevice currentDevice] systemVersion]]);
  });
  return *system_version;
}
#endif  // WEBRTC_IOS
}
}
