// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_PLUGINMANAGER_H_
#define OWT_BASE_PLUGINMANAGER_H_

#include "owt/ic/icmanagerinterface.h"

namespace owt {
namespace base {

/// Static class of managing dll plugins.
class PluginManager {
 public:
#if defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
  /**
    @brief Get the pointer of IC plugin. If loading owt_ic.dll is failed, a
    nullptr will be returned.
  */
  static owt::ic::ICManagerInterface* ICPlugin();
#endif

 private:
  PluginManager();
};
}  // namespace base
}  // namespace owt

#endif  // OWT_BASE_PLUGINMANAGER_H_
