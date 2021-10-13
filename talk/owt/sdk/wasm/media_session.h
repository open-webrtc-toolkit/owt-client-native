// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_WASM_MEDIASESSION_H_
#define OWT_WASM_MEDIASESSION_H_

#include "call/call.h"
#include "talk/owt/sdk/wasm/rtp_video_receiver.h"
#include "talk/owt/sdk/wasm/web_transport_session.h"
#include "third_party/webrtc/api/rtc_event_log/rtc_event_log_factory_interface.h"

namespace owt {
namespace wasm {
// Manages a media session between a web client and OWT server QUIC agent.
class MediaSession {
 public:
  explicit MediaSession();
  virtual ~MediaSession();
  std::shared_ptr<RtpVideoReceiver> CreateRtpVideoReceiver();

 private:
  std::unique_ptr<webrtc::TaskQueueFactory> task_queue_factory_;
  std::unique_ptr<webrtc::RtcEventLogFactoryInterface> event_log_factory_;
  std::unique_ptr<webrtc::RtcEventLog> event_log_;
  std::unique_ptr<WebTransportSession> web_transport_session_;
  std::unique_ptr<webrtc::Call> call_;
  std::unique_ptr<webrtc::ReceiveStatistics> receive_statistics_;
  std::unique_ptr<webrtc::ProcessThread> receiver_process_thread_;
  std::shared_ptr<RtpVideoReceiver> video_receiver_;
};

}  // namespace wasm
}  // namespace owt

#endif