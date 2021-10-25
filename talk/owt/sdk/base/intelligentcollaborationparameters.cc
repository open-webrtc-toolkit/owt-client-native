#include "talk/owt/sdk/include/cpp/owt/base/intelligentcollaborationparameters.h"

namespace owt {
namespace base {

void IntelligentCollaborationParameters::BackgroundBlur(bool enable) {
  enable_background_blur_ = enable;
}

void IntelligentCollaborationParameters::BlurRadius(int radius) {
  blur_radius_ = radius;
}

bool IntelligentCollaborationParameters::BackgroundBlur() const {
  return enable_background_blur_;
}

int IntelligentCollaborationParameters::BlurRadius() const {
  return blur_radius_;
}

}  // namespace base
}  // namespace owt
