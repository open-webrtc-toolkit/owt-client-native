// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_LOGGING_H_
#define OWT_BASE_LOGGING_H_
namespace owt {
namespace base {
enum class LoggingSeverity : int {
  /// Information which should only be logged with the consent of the user, due to privacy concerns.
  kSensitive = 1,
  /// This level is for data which we do not want to appear in the normal debug log, but should appear in diagnostic logs.
  kVerbose,
  /// Chatty level used in debugging for all sorts of things, the default in debug builds.
  kInfo,
  /// Something that may warrant investigation.
  kWarning,
  /// Something that should not have occurred.
  kError,
  /// Don't log.
  kNone
};
/// Logger configuration class. Choose either LogToConsole or LogToFileRotate in
/// your application for logging to console or file.
class Logging final {
 public:
  /// Set logging severity. All logging messages with higher severity will be
  /// logged.
  static void Severity(LoggingSeverity severity);
  /// Get current logging severity.
  static LoggingSeverity Severity();
  /// Set logging to console
  static void LogToConsole(LoggingSeverity severity);
  /// Set logging to files under provided dir rotately.
  static void LogToFileRotate(LoggingSeverity severity, std::string& dir, size_t max_log_size);
 private:
  static LoggingSeverity min_severity_;
};
}
}
#endif  // OWT_BASE_LOGGING_H
