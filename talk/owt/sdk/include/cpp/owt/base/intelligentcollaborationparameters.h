// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_INTELLIGENTCOLLABORATIONPARAMETERS_H_
#define OWT_BASE_INTELLIGENTCOLLABORATIONPARAMETERS_H_

namespace owt {
namespace base {

class IntelligentCollaborationParameters final {
 public:
  IntelligentCollaborationParameters() = default;
  ~IntelligentCollaborationParameters() = default;

  void BackgroundBlur(bool enable);
  void BlurRadius(int radius);

  bool BackgroundBlur() const;
  int BlurRadius() const;

 private:
  bool enable_background_blur_ = false;
  int blur_radius_ = 55;
};

}  // namespace base
}  // namespace owt

#endif  // OWT_BASE_INTELLIGENTCOLLABORATIONPARAMETERS_H_
