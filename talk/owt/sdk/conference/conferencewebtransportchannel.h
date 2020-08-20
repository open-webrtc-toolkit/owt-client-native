// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_CONFERENCE_CONFERENCEWEBTRANSPORTCHANNEL_H_
#define OWT_CONFERENCE_CONFERENCEWEBTRANSPORTCHANNEL_H_
#include <memory>
#include <mutex>
#include <unordered_map>
#include <chrono>
#include <random>
#include "webrtc/rtc_base/task_queue.h"
#include "talk/owt/include/sio_message.h"
#include "talk/owt/include/sio_client.h"
#include "talk/owt/sdk/conference/conferencesocketsignalingchannel.h"
#include "talk/owt/sdk/include/cpp/owt/base/exception.h"
#include "talk/owt/sdk/include/cpp/owt/base/stream.h"
#include "talk/owt/sdk/include/cpp/owt/conference/subscribeoptions.h"
#include "talk/owt/sdk/include/cpp/owt/conference/conferencepublication.h"
#include "talk/owt/sdk/include/cpp/owt/conference/conferenceclient.h"

namespace owt {

namespace quic {
class QuicTransportFactory;
class QuicTransportClientInterface;
}

namespace conference {
using namespace owt::base;

// An instance of ConferenceWebTransportChannel manages a WebTransport channel with
// MCU as well as it's signaling through Socket.IO.
class ConferenceWebTransportChannel: public owt::quic::QuicTransportClientInterface::Visitor, 
    public std::enable_shared_from_this<ConferenceWebTransportChannel> {
 public:
  explicit ConferenceWebTransportChannel(
      const std::string& url,
      std::shared_ptr<ConferenceSocketSignalingChannel> signaling_channel,
      std::shared_ptr<rtc::TaskQueue> event_queue);
  ~ConferenceWebTransportChannel();
  // Connect asynchronously to WebTransport server.
  void Connect();
  // Create WritableStream
  void CreateSendStream(
      std::function<void(std::shared_ptr<owt::base::WritableStream>)>
          on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  // Add a ConferenceWebTransportChannel observer so it will be notified when
  // this object have some events.
  void AddObserver(ConferenceWebTransportChannelObserver* observer);
  // Remove a ConferencePeerConnectionChannel observer. If the observer doesn't
  // exist, it will do nothing.
  void RemoveObserver();
  // Publish a local stream to the conference.
  void Publish(
      std::shared_ptr<LocalStream> stream,
      std::function<void(std::string)> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  // Unpublish a local stream to the conference.
  void Unpublish(
      const std::string& session_id,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  // Subscribe a stream from the conference.
  void Subscribe(
      std::shared_ptr<RemoteStream> stream,
      const SubscribeOptions& options,
      std::function<void(std::string)> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  // Unsubscribe a remote stream from the conference.
  void Unsubscribe(
      const std::string& session_id,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  // Stop current WebRTC session.
  void Stop(
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  // Get the associated stream id if it is a subscription channel.
  std::string GetSubStreamId();
  // Set stream's session ID. This ID is returned by MCU per publish/subscribe.
  void SetSessionId(const std::string& id);
  // Get published or subscribed stream's publicationID or subcriptionID.
  std::string GetSessionId() const;
  // Socket.IO event
  virtual void OnSignalingMessage(sio::message::ptr message);
  // TODO: define stats API
  void OnStreamError(const std::string& error_message);
 private:
  // Overrides owt::quic::QuicTransportClientInterface::Visitor
  void OnConnected();
  void OnConnectionFailed();
  void OnIncomingStream(QuicTransportStreamInterface*);
  bool CheckNullPointer(
      uintptr_t pointer,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  void SendStreamControlMessage(
      const std::string& in_action,
      const std::string& out_action,
      const std::string& operation,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure)
      const;
  void SendPublishMessage(
    sio::message::ptr options,
    std::shared_ptr<LocalStream> stream,
    std::function<void(std::unique_ptr<Exception>)> on_failure);
  std::function<void()> RunInEventQueue(std::function<void()> func);
  // Set publish_success_callback_, subscribe_success_callback_ and
  // failure_callback_ to nullptr.
  void ResetCallbacks();
  std::shared_ptr<ConferenceSocketSignalingChannel> signaling_channel_;
  std::string session_id_;   //session ID is 1:1 mapping to the subscribed/published stream.
  // If this pc is used for publishing, |local_stream_| will be the stream to be
  // published.
  // Otherwise, |remote_stream_| will be the stream to be subscribed.
  std::shared_ptr<RemoteStream> subscribed_stream_;
  std::shared_ptr<LocalStream> published_stream_;
  // Callbacks for publish or subscribe.
  std::function<void(std::string)> publish_success_callback_;
  std::function<void(std::string)> subscribe_success_callback_;
  std::function<void(std::unique_ptr<Exception>)> failure_callback_;
  std::mutex callback_mutex_;
  ConferenceWebTransportChannelObserver* observer_;
  bool connected_;
  // Mutex for firing subscription succeed callback.
  std::mutex sub_stream_added_mutex_;
  bool sub_stream_added_;
  bool sub_server_ready_;
  // Queue for callbacks and events.
  // Queue for callbacks and events.
  std::shared_ptr<rtc::TaskQueue> event_queue_;
  std::unique_ptr<QuicTransportFactory> quic_transport_factory_;
  std::unique_ptr<QuicTransportClientInterface> quic_transport_client_;
  // Each conference client will be associated with one quic_transport_channel_
  // instance.
  std::shared_ptr<ConferenceWebTransportChannel> quic_transport_channel_;
  std::atomic<bool> quic_client_connected_;
  std::string url_;

};
}
}
#endif  // OWT_CONFERENCE_CONFERENCEWEBTRANSPORTCHANNEL_H_
