// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/wasm/media_session.h"
#include <emscripten/threading.h>
#include "third_party/webrtc/api/rtc_event_log/rtc_event_log_factory.h"
#include "third_party/webrtc/api/transport/field_trial_based_config.h"
#include "third_party/webrtc/logging/rtc_event_log/fake_rtc_event_log_factory.h"
#include "third_party/webrtc/rtc_base/event.h"
#include "third_party/webrtc/rtc_base/task_queue_stdlib.h"

namespace owt {
namespace wasm {
MediaSession::MediaSession()
    : worker_thread_(rtc::Thread::Create()),
      task_queue_factory_(webrtc::CreateTaskQueueStdlibFactory()),
      event_log_factory_(std::make_unique<webrtc::FakeRtcEventLogFactory>()),
      event_log_(event_log_factory_->CreateRtcEventLog(
          webrtc::RtcEventLog::EncodingType::NewFormat)),
      web_transport_session_(std::make_unique<WebTransportSession>()),
      call_(nullptr),
      receive_statistics_(
          webrtc::ReceiveStatistics::Create(webrtc::Clock::GetRealTimeClock())),
      receiver_process_thread_(nullptr),
      video_receiver_(nullptr),
      rtcp_callback_(emscripten::val::null()) {
  RTC_CHECK(worker_thread_);
  worker_thread_->Start();
  worker_thread_->Invoke<void>(RTC_FROM_HERE, [&] {
    receiver_process_thread_ =
        webrtc::ProcessThread::Create("ReceiverProcessThread");
    webrtc::Call::Config config(event_log_.get());
    config.task_queue_factory = task_queue_factory_.get();
    webrtc::FieldTrialBasedConfig trials;
    config.trials = &trials;
    call_ = std::unique_ptr<webrtc::Call>(webrtc::Call::Create(config));
  });
}

std::shared_ptr<RtpVideoReceiver> MediaSession::CreateRtpVideoReceiver(
    uint32_t remote_ssrc) {
  return worker_thread_->Invoke<std::shared_ptr<RtpVideoReceiver>>(
      RTC_FROM_HERE, [&] {
        webrtc::VideoReceiveStream::Config config(this);
        config.rtp.remote_ssrc = remote_ssrc;
        config.rtp.local_ssrc = 1;
        // Same as `kNackRtpHistoryMs` in
        // third_party/webrtc/media/engine/webrtc_voice_engine.cc.
        config.rtp.nack.rtp_history_ms = 5000;
        video_receiver_ = std::make_shared<RtpVideoReceiver>(
            worker_thread_.get(), this, &config, receive_statistics_.get(),
            receiver_process_thread_.get());
        return video_receiver_;
      });
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
  emscripten_sync_run_in_main_runtime_thread(
      EM_FUNC_SIG_VIII, MediaSession::RunRtcpCallback, this, packet, length);
  return true;
}

void MediaSession::RunRtcpCallback(MediaSession* session,
                                   const uint8_t* packet,
                                   size_t length) {
  if (!session->rtcp_callback_) {
    return;
  }
  emscripten::val buffer(emscripten::typed_memory_view(length, packet));
  session->rtcp_callback_(buffer);
}

}  // namespace wasm
}  // namespace owt