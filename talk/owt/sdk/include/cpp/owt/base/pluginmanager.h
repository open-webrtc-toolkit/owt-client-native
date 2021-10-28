#ifndef OWT_BASE_PLUGINMANAGER_H_
#define OWT_BASE_PLUGINMANAGER_H_

#include "talk/owt/sdk/base/sharedobjectpointer.h"
#include "talk/owt/sdk/include/cpp/owt/ic/icmanagerinterface.h"

namespace owt {
namespace base {
class PluginManager {
 public:
  static PluginManager* GetInstance();

  owt::base::SharedObjectPointer<owt::ic::ICManagerInterface>& ICPlugin();

 private:
  PluginManager();

  owt::base::SharedObjectPointer<owt::ic::ICManagerInterface> ic_plugin;
};
}  // namespace base
}  // namespace owt

#endif  // OWT_BASE_PLUGINMANAGER_H_
