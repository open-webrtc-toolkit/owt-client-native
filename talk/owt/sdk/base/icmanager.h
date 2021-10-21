// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_ICMANAGER_H_
#define OWT_BASE_ICMANAGER_H_

#include <memory>
#ifdef WEBRTC_WIN
#include <windows.h>
#endif

#include "base/macros.h"
#include "webrtc/api/scoped_refptr.h"
#include "videoframepostprocessing.h"

namespace owt {
namespace base {

class ICManager {
 public:
  static ICManager* GetInstance();
  std::shared_ptr<VideoFramePostProcessing> CreatePostProcessor(
      const char* name);

 private:
  ICManager();
  ~ICManager();

  typedef owt::base::VideoFramePostProcessing* (*CREATE_POST_PROCESSING)(
      const char* name);
#ifdef WEBRTC_WIN
  HMODULE owt_ic_dll_ = nullptr;
#endif
  CREATE_POST_PROCESSING create_post_processing_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(ICManager);
};

}  // namespace base
}  // namespace owt

#endif  // OWT_BASE_ICMANAGER_H_
