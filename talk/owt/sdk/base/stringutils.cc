// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "talk/owt/sdk/base/stringutils.h"
#include "webrtc/rtc_base/third_party/base64/base64.h"
namespace owt {
namespace base {
bool StringUtils::IsBase64EncodedString(const std::string str) {
  for (size_t i = 0; i < str.size(); ++i) {
    if (!rtc::Base64::IsBase64Char(str.at(i)) && '=' != str.at(i)) {
      return false;
    }
  }
  return true;
}
}  // namespace base
}  // namespace owt
