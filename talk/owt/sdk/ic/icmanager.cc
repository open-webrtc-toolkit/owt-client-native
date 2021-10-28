// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/ic/icmanager.h"

#include "talk/owt/sdk/ic/backgroundblur.h"

namespace owt {
namespace ic {

bool ICManager::InitializeInferenceEngineCore(
    const std::string& plugins_xml_path) {
  return false;
}

std::shared_ptr<owt::base::VideoFramePostProcessor>
ICManager::CreatePostProcessor(ICPlugin plugin) {
  switch (plugin) {
    case ICPlugin::BACKGROUND_BLUR:
      return std::shared_ptr<owt::base::VideoFramePostProcessor>(
          new BackgroundBlur);
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
  delete reinterpret_cast<owt::ic::ICManager*>(ic_manager);
}
