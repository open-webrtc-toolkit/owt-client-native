/*
 * Intel License
 */

#ifndef WOOGEEN_BASE_SYSINFO_H_
#define WOOGEEN_BASE_SYSINFO_H_

#include <string>

namespace woogeen {
namespace base {
class SysInfo {
 public:
  // Returns the type of this SDK.
  static std::string SdkType();

  // Returns the version of this SDK.
  static std::string SdkVersion();
};
}
}

#endif  // WOOGEEN_BASE_SYSINFO_H_
