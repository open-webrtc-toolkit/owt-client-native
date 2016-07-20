/*
 * Intel License
 */

#include "talk/woogeen/sdk/base/sysinfo.h"

namespace woogeen {
namespace base {

static const std::string kVersion("3.2");

std::string SysInfo::SdkType() {
#if defined(WEBRTC_IOS) || defined(WEBRTC_MAC)
  return "Objective-C";
#else
  return "C++";
#endif
}

std::string SysInfo::SdkVersion() {
  return kVersion;
}
}
}
