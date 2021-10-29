// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/include/cpp/owt/ic/intelligentcollaborationparameters.h"
namespace owt {
namespace ic {

std::vector<std::shared_ptr<owt::base::VideoFramePostProcessor>>&
IntelligentCollaborationParameters::PostProcessors() {
  return post_processors_;
}

const std::vector<std::shared_ptr<owt::base::VideoFramePostProcessor>>&
IntelligentCollaborationParameters::PostProcessors() const {
  return post_processors_;
}

}  // namespace base
}  // namespace owt
