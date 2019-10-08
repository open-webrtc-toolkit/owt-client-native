// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_DEVICEUTILS_H_
#define OWT_BASE_DEVICEUTILS_H_
#include <vector>
#include <string>
#include "owt/base/commontypes.h"
namespace owt {
namespace base {
class DeviceUtils {
 public:
  /// Get video capturer IDs.
  static std::vector<std::string> VideoCapturerIds();
  /**
   Get supported resolutions for a specific video capturer.
   @param id The ID of specific video capturer.
   @return Supported resolution for the specific video capturer. If the name is
   invalid, it returns an empty vector.
   */
  static std::vector<Resolution> VideoCapturerSupportedResolutions(
      const std::string& id);
};
}
}
#endif  // OWT_BASE_DEVICEUTILS_H_
