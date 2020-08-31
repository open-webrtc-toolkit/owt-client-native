// Copyright (C) <2020> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

/*
 *  Copyright (c) 2020 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef OWT_BASE_WIN_VP9RATECONTROL_H_
#define OWT_BASE_WIN_VP9RATECONTROL_H_

#include <memory>
#include "webrtc/api/video/video_frame_type.h"

namespace libvpx {
struct VP9FrameParamsQpRTC;
struct VP9RateControlRtcConfig;
}  // namespace libvpx

namespace owt {
namespace base {

// Interface for QP & LF level computation for VP9.
class VP9RateControl {
 public:
  // Creates VP9RateControl using libvpx implementation.
  static std::unique_ptr<VP9RateControl> Create(
      const libvpx::VP9RateControlRtcConfig& config);

  virtual ~VP9RateControl() = default;

  virtual void UpdateRateControl(
      const libvpx::VP9RateControlRtcConfig& rate_control_config) = 0;

  virtual void ComputeQP(const libvpx::VP9FrameParamsQpRTC& frame_params) = 0;
  // QP range: 0-255.
  virtual int GetQP() const = 0;
  virtual int GetLoopfilterLevel() const = 0;
  virtual void PostEncodeUpdate(uint64_t encoded_frame_size) = 0;
};

}  // namespace base
}  // namespace owt
#endif  // OWT_BASE_WIN_VP9RATECONTROL_H_
