// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OMS_BASE_STRINGUTILS_H_
#define OMS_BASE_STRINGUTILS_H_
#include <string>
namespace oms {
namespace base {
/// This class provides utilities for string processing.
class StringUtils {
 public:
  /// Check if all characters are base 64 allowed or '='.
  static bool IsBase64EncodedString(const std::string str);
};
}
}
#endif  // OMS_BASE_STRINGUTILS_H_
