/*
 * Copyright © 2016 Intel Corporation. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ICS_BASE_LOGGING_H_
#define ICS_BASE_LOGGING_H_

namespace ics {
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

#endif  // WOOGEEN_BASE_LOGGING_H
