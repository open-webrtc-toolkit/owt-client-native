#ifndef OWT_BASE_PLUGINMANAGER_H_
#define OWT_BASE_PLUGINMANAGER_H_

#include "talk/owt/sdk/base/sharedobjectpointer.h"
#include "talk/owt/sdk/include/cpp/owt/ic/icmanagerinterface.h"

namespace owt {
namespace base {
class PluginManager {
 public:
  static PluginManager* GetInstance();

#if defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
  owt::base::SharedObjectPointer<owt::ic::ICManagerInterface>& ICPlugin();
#endif

 private:
  PluginManager();

#if defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
  owt::base::SharedObjectPointer<owt::ic::ICManagerInterface> ic_plugin;
#endif
};
}  // namespace base
}  // namespace owt

#endif  // OWT_BASE_PLUGINMANAGER_H_
