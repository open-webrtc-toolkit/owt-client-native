// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "icmanager.h"

#include "webrtc/rtc_base/logging.h"

namespace owt {
namespace base {

ICManager* ICManager::GetInstance() {
  static ICManager instance;
  return &instance;
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
    create_post_processing_ = reinterpret_cast<CREATE_POST_PROCESSING>(
        so_.GetSymbol("CreatePostProcessor"));
  } else {
    RTC_LOG(WARNING) << "owt_ic.dll is not loaded.";
  }
}

}  // namespace base
}  // namespace owt
