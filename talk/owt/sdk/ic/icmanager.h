// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_IC_ICMANAGER_H_
#define OWT_IC_ICMANAGER_H_

#include "talk/owt/sdk/include/cpp/owt/ic/icmanagerinterface.h"

#include <memory>

namespace InferenceEngine {
class Core;
}

namespace owt {
namespace ic {

class ICManager : public ICManagerInterface {
 public:
  ICManager();

  bool InitializeInferenceEngineCore(
      const std::string& plugins_xml_path) override;

  std::shared_ptr<owt::base::VideoFramePostProcessor> CreatePostProcessor(
      ICPlugin plugin) override;

 protected:
  std::shared_ptr<InferenceEngine::Core> core_;
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
