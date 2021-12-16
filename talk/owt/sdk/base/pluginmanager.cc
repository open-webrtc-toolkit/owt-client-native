// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/include/cpp/owt/base/pluginmanager.h"

#include "talk/owt/sdk/base/sharedobjectpointer.h"

#if defined(WEBRTC_WIN)
#define DLL_SUFFIX ".dll"
#elif defined(WEBRTC_LINUX)
#define DLL_SUFFIX ".so"
#endif

namespace owt {
namespace base {

#if defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
template <>
struct SOTrait<owt::ic::ICManagerInterface> {
  static constexpr auto name = "ICManager";
};

owt::ic::ICManagerInterface* PluginManager::ICPlugin() {
  static owt::base::SharedObjectPointer<owt::ic::ICManagerInterface> ic_plugin(
      "owt_ic" DLL_SUFFIX);
  return ic_plugin.Get();
}
#endif

}  // namespace base
}  // namespace owt
