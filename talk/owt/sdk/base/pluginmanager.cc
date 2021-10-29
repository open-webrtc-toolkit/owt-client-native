// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/include/cpp/owt/base/pluginmanager.h"

#include "talk/owt/sdk/base/sharedobjectpointer.h"

namespace owt {
namespace base {

template <>
struct SOTrait<owt::ic::ICManagerInterface> {
  static constexpr auto name = "ICManager";
};

#if defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
owt::ic::ICManagerInterface* PluginManager::ICPlugin() {
  static owt::base::SharedObjectPointer<owt::ic::ICManagerInterface> ic_plugin(
#if defined(WEBRTC_WIN)
      "owt_ic.dll"
#elif defined(WEBRTC_LINUX)
      "owt_ic.so"
#endif
  );
  return ic_plugin.Get();
}
#endif

}  // namespace base
}  // namespace owt
