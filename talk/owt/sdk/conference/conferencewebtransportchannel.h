// Copyright (C) <2020> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_CONFERENCE_CONFERENCEWEBTRANSPORTCHANNEL_H_
#define OWT_CONFERENCE_CONFERENCEWEBTRANSPORTCHANNEL_H_

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <random>
#include <string>
#include "webrtc/rtc_base/logging.h"
#include "webrtc/rtc_base/task_queue.h"
#include "talk/owt/include/sio_message.h"
#include "talk/owt/include/sio_client.h"
#include "talk/owt/sdk/conference/conferencesocketsignalingchannel.h"
#include "talk/owt/sdk/include/cpp/owt/base/exception.h"
#include "talk/owt/sdk/include/cpp/owt/base/stream.h"
#include "talk/owt/sdk/include/cpp/owt/conference/subscribeoptions.h"
#include "talk/owt/sdk/include/cpp/owt/conference/conferencepublication.h"
#include "talk/owt/sdk/include/cpp/owt/conference/conferenceclient.h"
#include "owt/quic/quic_transport_client_interface.h"
#include "owt/quic/quic_transport_factory.h"
#include "owt/quic/quic_transport_stream_interface.h"

namespace owt {
namespace conference {
using namespace owt::base;
using namespace owt::quic;

// An instance of ConferenceWebTransportChannel manages a WebTransport channel with
// MCU as well as its signaling through Socket.IO.
class ConferenceWebTransportChannel: public owt::quic::QuicTransportClientInterface::Visitor, 
    public std::enable_shared_from_this<ConferenceWebTransportChannel> {
 public:
  explicit ConferenceWebTransportChannel(
      const ConferenceClientConfiguration& configuration,
      const std::string& url,
      const std::string& webtransport_id,
      const std::string& webtransport_token,
      std::shared_ptr<ConferenceSocketSignalingChannel> signaling_channel,
      std::shared_ptr<rtc::TaskQueue> event_queue);
  ~ConferenceWebTransportChannel();
  class AuthStreamObserver
      : public owt::quic::QuicTransportStreamInterface::Visitor {
   public:
    AuthStreamObserver(ConferenceWebTransportChannel* channel)
        : channel_(channel) {}
    virtual void OnCanRead() {}
    virtual void OnCanWrite() {
      if (channel_ && !authenticated_) {
        channel_->Authenticate();
        authenticated_ = true;
      }
    }
    virtual void OnFinRead() {}
   private:
    ConferenceWebTransportChannel* channel_;
    bool authenticated_ = false;
  };
  class IncomingStreamObserver
      : public owt::quic::QuicTransportStreamInterface::Visitor {
   public:
    IncomingStreamObserver(ConferenceWebTransportChannel* channel, owt::quic::QuicTransportStreamInterface* stream)
        : channel_(channel)
        , stream_(stream) {}
    virtual void OnCanRead() {}
    virtual void OnCanWrite() {}
    virtual void OnFinRead() {}
   public:
    ConferenceWebTransportChannel* channel_;
    owt::quic::QuicTransportStreamInterface* stream_;
  };
  // Connect asynchronously to WebTransport server.
  void Connect();
  // Authenticate the channel.
  void Authenticate();
  // Create WritableStream
  void CreateSendStream(
      std::function<void(std::shared_ptr<owt::base::LocalStream>)>
          on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  // Add a ConferenceWebTransportChannel observer so it will be notified when
  // this object have some events.
  void AddObserver(ConferenceWebTransportChannelObserver* observer);
  // Remove a ConferencePeerConnectionChannel observer. If the observer doesn't
  // exist, it will do nothing.
  void RemoveObserver();
  // Publish a local stream to the conference.
  void Publish(std::shared_ptr<LocalStream> stream,
               std::function<void(std::string, std::string)> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  // Unpublish a local stream to the conference.
  void Unpublish(
      const std::string& session_id,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  // Subscribe a stream from the conference.
  void Subscribe(
      std::shared_ptr<RemoteStream> stream,
      std::function<void(std::string)> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  // Unsubscribe a remote stream from the conference.
  void Unsubscribe(
      const std::string& session_id,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  // Socket.IO event. WebTransport channel does not rely on this at
  // present. TODO(jianlin): handle quic signaling message for progress
  // update.
  virtual void OnSignalingMessage(sio::message::ptr message) {}
  // TODO: define stats API
  void OnStreamError(const std::string& error_message);
 private:
  // Implements owt::quic::QuicTransportClientInterface::Visitor
  void OnConnected();
  void OnConnectionFailed();
  void OnIncomingStream(QuicTransportStreamInterface*);
  void OnStreamSessionId(const std::string& session_id,
                         owt::quic::QuicTransportStreamInterface* stream);
  bool CheckNullPointer(
      uintptr_t pointer,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  void SendStreamControlMessage(
      const std::string& session_id,
      const std::string& in_action,
      const std::string& out_action,
      const std::string& operation,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure)
      const;
  void SendPublishMessage(
    sio::message::ptr options,
    std::shared_ptr<owt::base::LocalStream> stream,
    std::function <void(std::string session_id, std::string transport_id)> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure);
  std::function<void()> RunInEventQueue(std::function<void()> func);
  void AuthenticateCallback();
  ConferenceClientConfiguration configuration_;
  std::shared_ptr<ConferenceSocketSignalingChannel> signaling_channel_;
  ConferenceWebTransportChannelObserver* observer_;
  bool connected_;
  // Queue for callbacks and events.
  std::shared_ptr<rtc::TaskQueue> event_queue_;
  std::unique_ptr<owt::quic::QuicTransportFactory> quic_transport_factory_;
  std::unique_ptr<owt::quic::QuicTransportClientInterface> quic_transport_client_;
  // Each conference client will be associated with one quic_transport_channel_
  // instance.
  std::shared_ptr<ConferenceWebTransportChannel> quic_transport_channel_;
  std::atomic<bool> quic_client_connected_;
  std::string url_;
  std::string transport_id_;
  std::string webtransport_token_;  // base64 encoded webTransportToken recevied
                                    // after joining.
  QuicTransportStreamInterface* auth_stream_ = nullptr;
  std::unique_ptr<AuthStreamObserver> auth_stream_observer_;
  std::vector<std::string> published_session_ids_;
  mutable std::mutex published_session_ids_mutex_;
  std::vector<std::string> subscribed_session_ids_;
  mutable std::mutex subscribed_session_ids_mutex_;
  mutable std::mutex auth_mutex_;
  std::unique_ptr<std::condition_variable> auth_cv_;
};
}
}
#endif  // OWT_CONFERENCE_CONFERENCEWEBTRANSPORTCHANNEL_H_
