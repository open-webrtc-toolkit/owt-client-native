#ifndef OWT_IC_ICMANAGER_H_
#define OWT_IC_ICMANAGER_H_

#include "talk/owt/sdk/include/cpp/owt/ic/icmanagerinterface.h"

namespace owt {
namespace ic {

class ICManager : public ICManagerInterface {
 public:
  ICManager() = default;

  bool InitializeInferenceEngineCore(
      const std::string& plugins_xml_path) override;

  std::shared_ptr<owt::base::VideoFramePostProcessor> CreatePostProcessor(
      ICPlugin plugin) override;
};

}  // namespace ic
}  // namespace owt

#ifdef WEBRTC_WIN
#ifdef OWT_IC_IMPL
#define OWT_IC_EXPORT extern "C" __declspec(dllexport)
#else
#define OWT_IC_EXPORT extern "C" __declspec(dllimport)
#endif
#else
#define OWT_IC_EXPORT extern "C" __attribute__((visibility("default")))
#endif

OWT_IC_EXPORT owt::ic::ICManagerInterface* CreateICManager();

OWT_IC_EXPORT void DestroyICManager(owt::ic::ICManagerInterface* ic_manager);

#endif  // OWT_IC_ICMANAGER_H_
