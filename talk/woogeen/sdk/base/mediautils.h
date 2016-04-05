/*
 * Intel License
 */

#ifndef WOOGEEN_BASE_MEDIAUTILS_H_
#define WOOGEEN_BASE_MEDIAUTILS_H_

#include "talk/woogeen/sdk/include/cpp/woogeen/base/mediaformat.h"

namespace woogeen {
namespace base {
class MediaUtils {
 public:
  static std::string GetResolutionName(const Resolution& resolution);
};
}
}

#endif  // WOOGEEN_BASE_MEDIAUTILS_H_
