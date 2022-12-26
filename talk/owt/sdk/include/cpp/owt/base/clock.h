// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_CLOCK_H_
#define OWT_BASE_CLOCK_H_

#include <stdint.h>
#include "owt/base/export.h"

namespace webrtc {
class Clock;
}  // namespace webrtc

namespace owt {
namespace base {

/// A Wrapper of webrtc::Clock.
class OWT_EXPORT Clock {
 public:
  explicit Clock();
  int64_t TimeInMilliseconds();

 private:
   webrtc::Clock* clock_;
};

}  // namespace base
}  // namespace owt

#endif  // OWT_BASE_CLOCK_H_