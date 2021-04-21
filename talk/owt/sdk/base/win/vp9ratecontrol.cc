// Copyright (C) <2020> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include <memory>
#include "talk/owt/sdk/base/win/vp9ratecontrol.h"
#include "vp9/ratectrl_rtc.h"
#include "webrtc/rtc_base/logging.h"


namespace owt {
namespace base {

class LibvpxVP9RateControl : public VP9RateControl {
 public:
  explicit LibvpxVP9RateControl(std::unique_ptr<libvpx::VP9RateControlRTC> impl)
      : impl_(std::move(impl)) {}

  ~LibvpxVP9RateControl() override = default;
  LibvpxVP9RateControl(const LibvpxVP9RateControl&) = delete;
  LibvpxVP9RateControl& operator=(const LibvpxVP9RateControl&) = delete;

  void UpdateRateControl(
      const libvpx::VP9RateControlRtcConfig& rate_control_config) override {
    impl_->UpdateRateControl(rate_control_config);
  }
  int GetQP() const override { return impl_->GetQP(); }
  int GetLoopfilterLevel() const override {
    return impl_->GetLoopfilterLevel();
  }
  void ComputeQP(const libvpx::VP9FrameParamsQpRTC& frame_params) override {
    impl_->ComputeQP(frame_params);
  }
  void PostEncodeUpdate(uint64_t encoded_frame_size) override {
    impl_->PostEncodeUpdate(encoded_frame_size);
  }

 private:
  const std::unique_ptr<libvpx::VP9RateControlRTC> impl_;
};

// static
std::unique_ptr<VP9RateControl> VP9RateControl::Create(
    const libvpx::VP9RateControlRtcConfig& config) {
  auto impl = libvpx::VP9RateControlRTC::Create(config);
  if (!impl) {
    RTC_LOG(LS_ERROR) << "Failed creating libvpx::VP9RateControlRTC";
    return nullptr;
  }
  return std::make_unique<LibvpxVP9RateControl>(std::move(impl));
}
}  // namespace base
}  // namespace owt

