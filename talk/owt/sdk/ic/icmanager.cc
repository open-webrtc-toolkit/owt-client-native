// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/ic/icmanager.h"

#include "inference_engine.hpp"
#include "talk/owt/sdk/ic/backgroundblur.h"
#include "third_party/webrtc/rtc_base/logging.h"

namespace owt {
namespace ic {

ICManager::ICManager() : core_() {
  try {
    core_.reset(new InferenceEngine::Core);
  } catch (std::exception& e) {
    RTC_LOG(LS_ERROR) << "Failed to initialize inference engine core: "
                      << e.what();
  }
}

bool ICManager::RegisterInferenceEnginePlugins(
    const std::string& plugins_xml_path) {
  if (core_) {
    try {
      core_->RegisterPlugins(plugins_xml_path);
      return true;
    } catch (std::exception& e) {
      RTC_LOG(LS_ERROR) << "Failed to register inference engine plugins: "
                        << e.what();
    }
  }
  return false;
}

std::shared_ptr<owt::base::VideoFramePostProcessor>
ICManager::CreatePostProcessor(ICPostProcessor processor) {
  if (core_) {
    switch (processor) {
      case ICPostProcessor::BACKGROUND_BLUR:
        return std::make_shared<BackgroundBlur>(*core_);
      default:;
    }
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
