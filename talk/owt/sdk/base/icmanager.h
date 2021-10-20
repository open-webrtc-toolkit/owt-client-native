#ifndef OWT_BASE_ICMANAGER_H_
#define OWT_BASE_ICMANAGER_H_

#include <windows.h>
#include <memory>

#include "api/scoped_refptr.h"
#include "base/macros.h"
#include "videoframepostprocessing.h"

namespace owt {
namespace base {

class ICManager {
 public:
  static ICManager* GetInstance();
  std::shared_ptr<VideoFramePostProcessing> CreatePostProcessing(
      const char* name);

 private:
  ICManager();
  ~ICManager();

  typedef owt::base::VideoFramePostProcessing* (*CREATE_POST_PROCESSING)(
      const char* name);

  HMODULE owt_ic_dll_ = nullptr;
  CREATE_POST_PROCESSING create_post_processing_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(ICManager);
};

}  // namespace base
}  // namespace owt

#endif  // OWT_BASE_ICMANAGER_H_