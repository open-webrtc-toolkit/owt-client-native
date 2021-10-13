// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/wasm/media_session.h"
#include "third_party/webrtc/api/rtc_event_log/rtc_event_log_factory.h"
#include "third_party/webrtc/api/transport/field_trial_based_config.h"
#include "third_party/webrtc/rtc_base/task_queue_stdlib.h"

namespace owt {
namespace wasm {
MediaSession::MediaSession()
    : task_queue_factory_(webrtc::CreateTaskQueueStdlibFactory()),
      event_log_factory_(std::make_unique<webrtc::RtcEventLogFactory>(
          task_queue_factory_.get())),
      event_log_(event_log_factory_->CreateRtcEventLog(
          webrtc::RtcEventLog::EncodingType::NewFormat)),
      web_transport_session_(std::make_unique<WebTransportSession>()),
      call_(nullptr),
      receive_statistics_(
          webrtc::ReceiveStatistics::Create(webrtc::Clock::GetRealTimeClock())),
      receiver_process_thread_(nullptr),
      video_receiver_(nullptr) {
  RTC_LOG(LS_ERROR) << "Ctor of Media Session. " << __EMSCRIPTEN_PTHREADS__;
  rtc::ThreadManager::Instance()->WrapCurrentThread();
  receiver_process_thread_=webrtc::ProcessThread::Create("ReceiverProcessThread");
  webrtc::Call::Config config(event_log_.get());
  config.task_queue_factory = task_queue_factory_.get();
  webrtc::FieldTrialBasedConfig trials;
  config.trials = &trials;
  call_ = std::unique_ptr<webrtc::Call>(webrtc::Call::Create(config));
}

MediaSession::~MediaSession() {}

std::shared_ptr<RtpVideoReceiver> MediaSession::CreateRtpVideoReceiver() {
  webrtc::VideoReceiveStream::Config config(web_transport_session_.get());
  config.rtp.local_ssrc = 1;
  // TODO: Get a message queue for receiver.
  video_receiver_ = std::make_shared<RtpVideoReceiver>(
      web_transport_session_.get(), &config, receive_statistics_.get(),
      receiver_process_thread_.get(), web_transport_session_.get());
  return video_receiver_;
}

}  // namespace wasm
}  // namespace owt