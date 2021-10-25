// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "icmanager.h"

#include <fstream>

#include "third_party/jsoncpp/source/include/json/json.h"
#include "webrtc/rtc_base/logging.h"

namespace owt {
namespace base {

ICManager* ICManager::GetInstance() {
  static ICManager instance;
  return &instance;
}

bool ICManager::LoadConfig(const std::string& config_path) {
  Json::Reader reader;
  Json::Value config;
  std::ifstream fin(config_path);
  if (!fin) {
    return false;
  }
  if (!reader.parse(fin, config)) {
    return false;
  }
  Json::Value backgroundBlur = config.get("background_blur", Json::Value());
  Json::Value bins = backgroundBlur.get("bin", Json::Value());
  for (auto& bin : bins) {
    std::string source = bin.get("source", "").asString();
    std::string target = bin.get("target", "").asString();
    // TODO WIP
  }
}

std::shared_ptr<VideoFramePostProcessor> ICManager::CreatePostProcessor(
    const char* name) {
  return create_post_processing_
             ? std::shared_ptr<VideoFramePostProcessor>(
                   create_post_processing_(name),
                   [](VideoFramePostProcessor* ptr) { ptr->Release(); })
             : nullptr;
}

ICManager::ICManager()
    : so_(
#ifdef WEBRTC_WIN
          "owt_ic.dll"
#elif WEBRTC_LINUX
          "owt_ic.so"
#endif
      ) {
  if (so_.IsLoaded()) {
    RTC_LOG(INFO) << "owt_ic.dll is loaded.";
    create_post_processing_ = reinterpret_cast<CREATE_POST_PROCESSOR>(
        so_.GetSymbol("CreatePostProcessor"));
    LoadConfig("config/owt_ic.json");
  } else {
    RTC_LOG(WARNING) << "owt_ic.dll is not loaded.";
  }
}

}  // namespace base
}  // namespace owt
