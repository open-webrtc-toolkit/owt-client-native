// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "talk/owt/sdk/include/cpp/owt/base/clock.h"
#include "webrtc/system_wrappers/include/clock.h"

namespace owt {
namespace base {

Clock::Clock() : clock_(webrtc::Clock::GetRealTimeClock()) {}

int64_t Clock::TimeInMilliseconds() {
  return clock_->TimeInMilliseconds();
}

}  // namespace base
}  // namespace owt