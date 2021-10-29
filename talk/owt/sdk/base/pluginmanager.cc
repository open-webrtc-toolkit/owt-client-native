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

PluginManager::PluginManager()
#if defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
    : ic_plugin(
#ifdef WEBRTC_WIN
          "owt_ic.dll"
#elif WEBRTC_LINUX
          "owt_ic.so"
#endif
      )
#endif
{
}

}  // namespace base
}  // namespace owt
