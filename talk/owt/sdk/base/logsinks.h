/*
*  Copyright 2015 The WebRTC Project Authors. All rights reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/
// This file is borrowed from webrtc/rtc_base/logsinks.h
#ifndef OWT_BASE_LOGSINKS_H_
#define OWT_BASE_LOGSINKS_H_
#include <memory>
#include <string>
#include "webrtc/rtc_base/constructor_magic.h"
#include "webrtc/rtc_base/file_rotating_stream.h"
#include "webrtc/rtc_base/logging.h"
namespace owt {
namespace base {
using namespace rtc;
// Log sink that uses a FileRotatingStream to write to disk.
// Init() must be called before adding this sink.
class RotatingLogSink : public LogSink {
 public:
  // |num_log_files| must be greater than 1 and |max_log_size| must be greater
  // than 0.
  RotatingLogSink(const std::string& log_dir_path,
                      const std::string& log_prefix,
                      size_t max_log_size,
                      size_t num_log_files);
  ~RotatingLogSink() override;
  // Writes the message to the current file. It will spill over to the next
  // file if needed.
  void OnLogMessage(const std::string& message) override;
  // Deletes any existing files in the directory and creates a new log file.
  virtual bool Init();
  // Disables buffering on the underlying stream.
  bool DisableBuffering();
 protected:
  explicit RotatingLogSink(FileRotatingStream* stream);
 private:
  std::unique_ptr<FileRotatingStream> stream_;
  RTC_DISALLOW_COPY_AND_ASSIGN(RotatingLogSink);
};
// Log sink that uses a OWTFileRotatingStream to write to disk.
// Init() must be called before adding this sink.
class CallSessionRotatingLogSink : public RotatingLogSink {
 public:
  ~CallSessionRotatingLogSink() override;
  static std::shared_ptr<CallSessionRotatingLogSink> Create(const std::string& log_dir_path,
      size_t max_total_log_size) {
      static std::shared_ptr<CallSessionRotatingLogSink> s(new CallSessionRotatingLogSink(log_dir_path, max_total_log_size));
      return s;
  }
 private:
  CallSessionRotatingLogSink(const std::string& log_dir_path,
                         size_t max_total_log_size);
  RTC_DISALLOW_COPY_AND_ASSIGN(CallSessionRotatingLogSink);
};
}  // namespace base
}  // namespace owt
#endif  // OWT_BASE_LOGSINKS_H_
