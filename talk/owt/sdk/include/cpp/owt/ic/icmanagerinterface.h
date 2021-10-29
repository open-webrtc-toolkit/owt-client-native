// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_IC_ICMANAGERINTERFACE_H_
#define OWT_IC_ICMANAGERINTERFACE_H_

#include <memory>
#include <string>

#include "owt/base/videoframepostprocessor.h"

namespace owt {
namespace ic {

enum class ICPlugin { BACKGROUND_BLUR };

class ICManagerInterface {
 public:
  virtual bool InitializeInferenceEngineCore(
      const std::string& plugins_xml_path) = 0;

  virtual std::shared_ptr<owt::base::VideoFramePostProcessor>
  CreatePostProcessor(ICPlugin plugin) = 0;

 protected:
  ~ICManagerInterface() = default;
};

}  // namespace ic
}  // namespace owt

#endif  // OWT_IC_ICMANAGERINTERFACE_H_
