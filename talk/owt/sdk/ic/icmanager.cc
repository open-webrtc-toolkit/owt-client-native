// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/ic/icmanager.h"

#include "inference_engine.hpp"
#include "talk/owt/sdk/ic/backgroundblur.h"
#include "third_party/webrtc/rtc_base/logging.h"

namespace owt {
namespace ic {

bool ICManager::RegisterInferenceEnginePlugins(
    const std::string& plugins_xml_path) {
  if (!core_) {
    try {
      core_.reset(new InferenceEngine::Core(plugins_xml_path));
    } catch (const std::exception& e) {
      RTC_LOG(LS_ERROR) << "Cannot initialize inference engine core: "
                        << e.what();
      return false;
    }
  }
  try {
    core_->RegisterPlugins(plugins_xml_path);
  } catch (const std::exception& e) {
    RTC_LOG(LS_ERROR) << "Cannot register inference engine plugins: "
                      << e.what();
    return false;
  }
  return true;
}

std::shared_ptr<owt::base::VideoFramePostProcessor>
ICManager::CreatePostProcessor(ICPostProcessor processor) {
  if (!core_ && !RegisterInferenceEnginePlugins({})) {
    return nullptr;
  }
  switch (processor) {
    case ICPostProcessor::BACKGROUND_BLUR:
      return std::make_shared<BackgroundBlur>(*core_);
    default:
      RTC_LOG(LS_WARNING) << "CreatePostProcessor(" << processor
                          << ") is not implemented.";
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
