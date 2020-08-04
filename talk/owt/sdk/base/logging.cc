// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include <unordered_map>
#include <string>
#include "talk/owt/sdk/include/cpp/owt/base/logging.h"
#include "webrtc/rtc_base/logging.h"
#include "talk/owt/sdk/base/logsinks.h"
namespace owt {
namespace base {
#if !defined(NDEBUG)
LoggingSeverity Logging::min_severity_ = LoggingSeverity::kInfo;
#else
LoggingSeverity Logging::min_severity_ = LoggingSeverity::kNone;
#endif
// Due to a defect in C++ 11, static cast to int instead of enum value.
// http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#2148
static std::unordered_map<int, rtc::LoggingSeverity> logging_severity_map = {
    {static_cast<int>(LoggingSeverity::kVerbose), rtc::LS_VERBOSE},
    {static_cast<int>(LoggingSeverity::kInfo), rtc::LS_INFO},
    {static_cast<int>(LoggingSeverity::kWarning), rtc::LS_WARNING},
    {static_cast<int>(LoggingSeverity::kError), rtc::LS_ERROR},
    {static_cast<int>(LoggingSeverity::kNone), rtc::LS_NONE}};
static std::unordered_map<int, std::string> logging_param_map = {
    { static_cast<int>(LoggingSeverity::kVerbose), "verbose" },
    { static_cast<int>(LoggingSeverity::kInfo), "info" },
    { static_cast<int>(LoggingSeverity::kWarning), "warning" },
    { static_cast<int>(LoggingSeverity::kError), "error" },
    { static_cast<int>(LoggingSeverity::kNone), "none" } };
void Logging::Severity(LoggingSeverity severity) {
  min_severity_ = severity;
  rtc::LogMessage::LogToDebug(logging_severity_map[static_cast<int>(severity)]);
}
void Logging::LogToConsole(LoggingSeverity severity) {
  min_severity_ = severity;
  rtc::LogMessage::ConfigureLogging(logging_param_map[static_cast<int>(severity)].c_str());
}
void Logging::LogToFileRotate(LoggingSeverity severity, std::string& dir, size_t max_log_size) {
  min_severity_ = severity;
  std::shared_ptr<CallSessionRotatingLogSink> log_sink = CallSessionRotatingLogSink::Create(dir, max_log_size);
  log_sink->Init();
  rtc::LogMessage::AddLogToStream(log_sink.get(), logging_severity_map[static_cast<int>(severity)]);
}
LoggingSeverity Logging::Severity() {
  return min_severity_;
}
}
}
