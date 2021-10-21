// Copyright (C) <2018> Intel Corporation
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

std::shared_ptr<VideoFramePostProcessing> ICManager::CreatePostProcessing(
    const char* name) {
  return create_post_processing_
             ? std::shared_ptr<VideoFramePostProcessing>(
                   create_post_processing_(name),
                   [](VideoFramePostProcessing* ptr) { ptr->Release(); })
             : nullptr;
}

ICManager::ICManager() {
  if (owt_ic_dll_ = LoadLibrary(L"owt_ic.dll")) {
    RTC_LOG(INFO) << "owt_ic.dll is loaded.";
    create_post_processing_ = (CREATE_POST_PROCESSING)GetProcAddress(
        owt_ic_dll_, "CreatePostProcessing");
  } else {
    RTC_LOG(WARNING) << "owt_ic.dll is not loaded.";
  }
}

ICManager::~ICManager() {
  if (owt_ic_dll_) {
    FreeLibrary(owt_ic_dll_);
  }
}

}  // namespace base
}  // namespace owt
