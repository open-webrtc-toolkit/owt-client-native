// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_IC_INTELLIGENTCOLLABORATIONPARAMETERS_H_
#define OWT_IC_INTELLIGENTCOLLABORATIONPARAMETERS_H_

#include <memory>
#include <vector>

#include "talk/owt/sdk/include/cpp/owt/base/videoframepostprocessor.h"

namespace owt {
namespace ic {

class IntelligentCollaborationParameters final {
 public:
  IntelligentCollaborationParameters() = default;
  ~IntelligentCollaborationParameters() = default;

  std::vector<std::shared_ptr<owt::base::VideoFramePostProcessor>>&
  PostProcessors();
  const std::vector<std::shared_ptr<owt::base::VideoFramePostProcessor>>&
  PostProcessors() const;

 private:
  std::vector<std::shared_ptr<owt::base::VideoFramePostProcessor>>
      post_processors_;
};

}  // namespace ic
}  // namespace owt

#endif  // OWT_IC_INTELLIGENTCOLLABORATIONPARAMETERS_H_
