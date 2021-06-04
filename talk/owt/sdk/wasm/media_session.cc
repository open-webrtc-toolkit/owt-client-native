// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/wasm/media_session.h"

namespace owt {
namespace wasm {
MediaSession::MediaSession()
    : call_(std::unique_ptr<webrtc::Call>(
          webrtc::Call::Create(webrtc::Call::Config(nullptr)))),
      video_receiver_(nullptr) {}

MediaSession::~MediaSession() {}

RtpVideoReceiver* MediaSession::CreateRtpVideoReceiver() {
  webrtc::VideoReceiveStream::Config config(web_transport_session_.get());
  webrtc::TaskQueueBase* task_queue = webrtc::TaskQueueBase::Current();
  if (!task_queue) {
    task_queue = rtc::ThreadManager::Instance()->CurrentThread();
  }
  std::unique_ptr<webrtc::RtpVideoStreamReceiver2> receiver =
      std::make_unique<webrtc::RtpVideoStreamReceiver2>(
          task_queue, webrtc::Clock::GetRealTimeClock(),
          web_transport_session_.get(), nullptr, nullptr, &config, nullptr,
          nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
          nullptr);
  video_receiver_ = std::make_unique<RtpVideoReceiver>(std::move(receiver));
  return video_receiver_.get();
}
}  // namespace wasm
}  // namespace owt