// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_ICMANAGERINTERFACE_H_
#define OWT_BASE_ICMANAGERINTERFACE_H_

#include <memory>
#include <string>

#include "talk/owt/sdk/include/cpp/owt/base/videoframepostprocessor.h"

namespace owt {
namespace base {

    enum class ICPlugin {
        BACKGROUND_BLUR,
    };

class ICManagerInterface {
 public:
  virtual void InitializeInferenceEngineCore(const std::string &plugin_xml_path);

  virtual std::shared_ptr<VideoFramePostProcessor> CreatePostProcessor(
      ICPlugin plugin);
};

}  // namespace base
}  // namespace owt

#endif  // OWT_BASE_ICMANAGERINTERFACE_H_
