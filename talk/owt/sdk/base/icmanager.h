// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_ICMANAGER_H_
#define OWT_BASE_ICMANAGER_H_

#include <memory>
#include <string>

#include "base/macros.h"
#include "talk/owt/sdk/base/sharedobjectloader.h"
#include "talk/owt/sdk/base/videoframepostprocessing.h"
#include "webrtc/api/scoped_refptr.h"

namespace owt {
namespace base {

class ICManager {
 public:
  static ICManager* GetInstance();

  bool LoadConfig(const std::string& config_path);
  std::shared_ptr<VideoFramePostProcessor> CreatePostProcessor(
      const char* name);

 private:
  ICManager();
  ~ICManager() = default;

  typedef owt::base::VideoFramePostProcessor* (*CREATE_POST_PROCESSOR)(
      const char* name);
  SharedObjectLoader so_;
  CREATE_POST_PROCESSOR create_post_processing_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(ICManager);
};

}  // namespace base
}  // namespace owt

#endif  // OWT_BASE_ICMANAGER_H_
