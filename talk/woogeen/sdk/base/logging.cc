/*
 * Intel License
 */

#include <unordered_map>
#include "talk/woogeen/sdk/include/cpp/woogeen/base/logging.h"
#include "webrtc/rtc_base/logging.h"

namespace woogeen {
namespace base {

#if !defined(NDEBUG)
LoggingSeverity Logging::min_severity_ = LoggingSeverity::kInfo;
#else
LoggingSeverity Logging::min_severity_ = LoggingSeverity::kNone;
#endif

// Due to a defect in C++ 11, static cast to int instead of enum value.
// http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#2148
static std::unordered_map<int, rtc::LoggingSeverity> logging_severity_map = {
    {static_cast<int>(LoggingSeverity::kSensitive), rtc::LS_SENSITIVE},
    {static_cast<int>(LoggingSeverity::kVerbose), rtc::LS_VERBOSE},
    {static_cast<int>(LoggingSeverity::kInfo), rtc::LS_INFO},
    {static_cast<int>(LoggingSeverity::kWarning), rtc::LS_WARNING},
    {static_cast<int>(LoggingSeverity::kError), rtc::LS_ERROR},
    {static_cast<int>(LoggingSeverity::kNone), rtc::LS_NONE}};

void Logging::Severity(LoggingSeverity severity) {
  min_severity_ = severity;
  rtc::LogMessage::LogToDebug(logging_severity_map[static_cast<int>(severity)]);
}

LoggingSeverity Logging::Severity() {
  return min_severity_;
}

}
}
