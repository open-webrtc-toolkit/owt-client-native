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

/// The type of IC post processor
enum class ICPostProcessor { BACKGROUND_BLUR };

/// The IC plugin manager, which creates the post processor instance.
class ICManagerInterface {
 public:
  virtual ~ICManagerInterface() = default;

  /**
    @brief Register inference engine plugins.
    @param plugins_xml_path The path to plugins.xml of OpenVINO Inference
    Engine.
    @return Whether the process succeeds. In case of failure, the error message
    will be printed to RTC log.
  */
  virtual bool RegisterInferenceEnginePlugins(
      const std::string& plugins_xml_path) = 0;

  /**
    @brief Create and get the IC post processor.
    @param processor The type of processor to be created.
    @return The shared pointer of the post processor.
  */
  virtual std::shared_ptr<owt::base::VideoFramePostProcessor>
  CreatePostProcessor(ICPostProcessor processor) = 0;
};

}  // namespace ic
}  // namespace owt

#endif  // OWT_IC_ICMANAGERINTERFACE_H_
