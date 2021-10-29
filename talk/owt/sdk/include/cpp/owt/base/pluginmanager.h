// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_PLUGINMANAGER_H_
#define OWT_BASE_PLUGINMANAGER_H_

#include "owt/ic/icmanagerinterface.h"

namespace owt {
namespace base {

class PluginManager {
 public:
#if defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
  static owt::ic::ICManagerInterface* ICPlugin();
#endif

 private:
  PluginManager();
};
}  // namespace base
}  // namespace owt

#endif  // OWT_BASE_PLUGINMANAGER_H_
