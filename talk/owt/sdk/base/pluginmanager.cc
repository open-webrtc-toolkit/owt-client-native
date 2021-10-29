// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "..\include\cpp\owt\base\pluginmanager.h"

namespace owt {
namespace base {

PluginManager* PluginManager::GetInstance() {
  static PluginManager plugin_manager;
  return &plugin_manager;
}

#if defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
owt::base::SharedObjectPointer<owt::ic::ICManagerInterface>&
PluginManager::ICPlugin() {
  return ic_plugin;
}
#endif

#if defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
PluginManager::PluginManager()
    : ic_plugin(
#if defined(WEBRTC_WIN)
          "owt_ic.dll"
#elif defined(WEBRTC_LINUX)
          "owt_ic.so"
#endif
      ) {
}
#else
PluginManager::PluginManager() {}
#endif

}  // namespace base
}  // namespace owt
