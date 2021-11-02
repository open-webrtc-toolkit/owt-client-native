// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/ic/icmanager.h"

#include "inference_engine.hpp"
#include "talk/owt/sdk/ic/backgroundblur.h"

namespace owt {
namespace ic {
ICManager::ICManager() : core(new InferenceEngine::Core()) {
}

bool ICManager::InitializeInferenceEngineCore(
    const std::string& plugins_xml_path) {
  try {
    core->RegisterPlugins(plugins_xml_path);
    return true;
  } catch (std::exception &) {
    return false;
  }
}

std::shared_ptr<owt::base::VideoFramePostProcessor>
ICManager::CreatePostProcessor(ICPlugin plugin) {
  switch (plugin) {
    case ICPlugin::BACKGROUND_BLUR:
      return std::make_shared<BackgroundBlur>(*core);
    default:;
  }
  return nullptr;
}

}  // namespace ic
}  // namespace owt

owt::ic::ICManagerInterface* CreateICManager() {
  return new owt::ic::ICManager;
}

void DestroyICManager(owt::ic::ICManagerInterface* ic_manager) {
  delete ic_manager;
}
