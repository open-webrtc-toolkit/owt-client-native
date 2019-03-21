/*
*  Copyright 2015 The WebRTC Project Authors. All rights reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/
// This file is borrowed from webrtc/rtc_base/logsinks.cc
#include "talk/owt/sdk/base/logsinks.h"
#include <iostream>
#include <string>
#include "webrtc/rtc_base/checks.h"
namespace owt {
namespace base {
using namespace rtc;
RotatingLogSink::RotatingLogSink(const std::string& log_dir_path,
                                         const std::string& log_prefix,
                                         size_t max_log_size,
                                         size_t num_log_files)
    : RotatingLogSink(new FileRotatingStream(log_dir_path,
                                                 log_prefix,
                                                 max_log_size,
                                                 num_log_files)) {
}
RotatingLogSink::RotatingLogSink(FileRotatingStream* stream)
    : stream_(stream) {
  RTC_DCHECK(stream);
}
RotatingLogSink::~RotatingLogSink() {
}
void RotatingLogSink::OnLogMessage(const std::string& message) {
  if (stream_->GetState() != SS_OPEN) {
    std::cerr << "Init() must be called before adding this sink." << std::endl;
    return;
  }
  stream_->WriteAll(message.c_str(), message.size(), nullptr, nullptr);
}
bool RotatingLogSink::Init() {
  return stream_->Open();
}
bool RotatingLogSink::DisableBuffering() {
  return stream_->DisableBuffering();
}
CallSessionRotatingLogSink::CallSessionRotatingLogSink(
    const std::string& log_dir_path,
    size_t max_total_log_size)
    : RotatingLogSink(
          new CallSessionFileRotatingStream(log_dir_path, max_total_log_size)) {
}
CallSessionRotatingLogSink::~CallSessionRotatingLogSink() {
}
}  // namespace base
}  // namespace owt
