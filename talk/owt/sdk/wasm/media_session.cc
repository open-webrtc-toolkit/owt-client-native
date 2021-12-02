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
      video_receiver_(nullptr),
      rtcp_callback_(emscripten::val::null()) {
  rtc::ThreadManager::Instance()->WrapCurrentThread();
  receiver_process_thread_ =
      webrtc::ProcessThread::Create("ReceiverProcessThread");
  webrtc::Call::Config config(event_log_.get());
  config.task_queue_factory = task_queue_factory_.get();
  webrtc::FieldTrialBasedConfig trials;
  config.trials = &trials;
  call_ = std::unique_ptr<webrtc::Call>(webrtc::Call::Create(config));
}

std::shared_ptr<RtpVideoReceiver> MediaSession::CreateRtpVideoReceiver() {
  webrtc::VideoReceiveStream::Config config(this);
  config.rtp.local_ssrc = 1;
  // Same as `kNackRtpHistoryMs` in third_party/webrtc/media/engine/webrtc_voice_engine.cc.
  config.rtp.nack.rtp_history_ms = 5000;
  // TODO: Use call_->worker_thread() when libwebrtc is rolled to a newer version.
  webrtc::TaskQueueBase* current = webrtc::TaskQueueBase::Current();
  if (!current)
    current = rtc::ThreadManager::Instance()->CurrentThread();
  RTC_DCHECK(current);
  video_receiver_ = std::make_shared<RtpVideoReceiver>(
      current, this, &config, receive_statistics_.get(),
      receiver_process_thread_.get());
  return video_receiver_;
}

void MediaSession::SetRtcpCallback(emscripten::val callback) {
  rtcp_callback_ = callback;
}

bool MediaSession::SendRtp(const uint8_t* packet,
                           size_t length,
                           const webrtc::PacketOptions& options) {
  return false;
}

bool MediaSession::SendRtcp(const uint8_t* packet, size_t length) {
  if (!rtcp_callback_) {
    return false;
  }
  emscripten::val buffer(emscripten::typed_memory_view(length, packet));
  rtcp_callback_(buffer);
  return true;
}

}  // namespace wasm
}  // namespace owt