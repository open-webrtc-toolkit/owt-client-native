#include "..\include\cpp\owt\base\pluginmanager.h"

namespace owt {
namespace base {

PluginManager* PluginManager::GetInstance() {
  static PluginManager plugin_manager;
  return &plugin_manager;
}

owt::base::SharedObjectPointer<owt::ic::ICManagerInterface>&
PluginManager::ICPlugin() {
  return ic_plugin;
}

PluginManager::PluginManager()
    : ic_plugin(
#ifdef WEBRTC_WIN
          "owt_ic.dll"
#elif WEBRTC_LINUX
          "owt_ic.so"
#endif
      ) {
}

}  // namespace base
}  // namespace owt
