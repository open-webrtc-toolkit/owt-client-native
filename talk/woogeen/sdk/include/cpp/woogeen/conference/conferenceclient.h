/*
 * Copyright © 2015 Intel Corporation. All Rights Reserved.
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

#ifndef WOOGEEN_CONFERENCE_CONFERENCECLIENT_H_
#define WOOGEEN_CONFERENCE_CONFERENCECLIENT_H_

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>
#include "woogeen/base/clientconfiguration.h"
#include "woogeen/base/globalconfiguration.h"
#include "woogeen/base/stream.h"
#include "woogeen/conference/user.h"
#include "woogeen/conference/conferenceexception.h"
#include "woogeen/conference/subscribeoptions.h"

namespace sio{
  class message;
}

namespace woogeen {
namespace base {
  struct PeerConnectionChannelConfiguration;
}
}

namespace woogeen {
namespace conference {

using namespace woogeen::base;

/**
  @brief Configuration for creating a ConferenceClient
  @detail This configuration is used while creating ConferenceClient.
  Changing this configuration does NOT impact ConferenceClient already
  created.
*/
struct ConferenceClientConfiguration : ClientConfiguration {};

class RemoteMixedStream;
class ConferencePeerConnectionChannel;
class ConferenceSocketSignalingChannel;

/** @cond */
class ConferenceSocketSignalingChannelObserver {
 public:
  virtual void OnStreamAdded(std::shared_ptr<sio::message> stream) = 0;
  virtual void OnUserJoined(
      std::shared_ptr<const woogeen::conference::User> user) = 0;
  virtual void OnUserLeft(
      std::shared_ptr<const woogeen::conference::User> user) = 0;
  virtual void OnStreamRemoved(std::shared_ptr<sio::message> stream) = 0;
  virtual void OnServerDisconnected() = 0;
  virtual void OnCustomMessage(std::string& from, std::string& message) = 0;
  virtual void OnSignalingMessage(std::shared_ptr<sio::message> message) = 0;
  // Notify the ID for a published stream.
  virtual void OnStreamId(const std::string& id, const std::string& label) = 0;
};
/** @endcond */

/// Observer for RTCConferenceClient.
class ConferenceClientObserver {
 public:
  /**
    @brief Triggers when a stream is added.
    @param stream The stream which is added.
  */
  virtual void OnStreamAdded(
      std::shared_ptr<RemoteCameraStream> stream){};
  /**
    @brief Triggers when a stream is added.
    @param stream The stream which is added.
  */
  virtual void OnStreamAdded(
      std::shared_ptr<RemoteScreenStream> stream){};
  /**
    @brief Triggers when a stream is added.
    @param stream The stream which is added.
  */
  virtual void OnStreamAdded(
      std::shared_ptr<RemoteMixedStream> stream){};
  /**
    @brief Triggers when a stream is removed.
    @param stream The stream which is removed.
  */
  virtual void OnStreamRemoved(
      std::shared_ptr<RemoteCameraStream> stream){};
  /**
    @brief Triggers when a stream is removed.
    @param stream The stream which is removed.
  */
  virtual void OnStreamRemoved(
      std::shared_ptr<RemoteScreenStream> stream){};
  /**
    @brief Triggers when a stream is removed.
    @param stream The stream which is removed.
  */
  virtual void OnStreamRemoved(
      std::shared_ptr<RemoteMixedStream> stream){};
  /**
    @brief Triggers when a message is received.
    @param sender_id Sender's ID.
    @param message Message received.
  */
  virtual void OnMessageReceived(std::string& sender_id,
                                 std::string& message){};
  /**
    @brief Triggers when a user joined conference.
    @param user The user joined.
  */
  virtual void OnUserJoined(std::shared_ptr<const conference::User>){};
  /**
    @brief Triggers when a user left conference.
    @param user The user left.
  */
  virtual void OnUserLeft(std::shared_ptr<const conference::User>){};
  /// Triggered when server is disconnected.
  virtual void OnServerDisconnected(){};
};

/// An asynchronous class for app to communicate with a conference in MCU.
class ConferenceClient final : ConferenceSocketSignalingChannelObserver {
 public:
  /**
    @brief Initialize a ConferenceClient instance with specific configuration
    @param config Configuration for creating the ConferenceClient.
  */
  ConferenceClient(const ConferenceClientConfiguration& configuration);
  /// Add an observer for conferenc client.
  void AddObserver(ConferenceClientObserver& observer);
  /// Remove an object from conference client.
  void RemoveObserver(ConferenceClientObserver& observer);
  /**
    @brief Connect to the specified room to join a conference.
    @param token Includes the room info which is encrypted.
  */
  void Join(
      const std::string& token,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  /**
    @brief Publish the stream to the current room.
    @param stream The stream to be published.
  */
  void Publish(
      std::shared_ptr<LocalStream> stream,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  /**
    @brief Subscribe a stream from the current room.
    @param stream The remote stream to be subscribed.
    @param onSuccess Success callback with a stream that contains media stream.
  */
  void Subscribe(
      std::shared_ptr<RemoteStream> stream,
      std::function<void(std::shared_ptr<RemoteStream> stream)> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  /**
    @brief Subscribe a stream from the current room.
    @param stream The remote stream to be subscribed.
    @param options Options for subscribing the stream.
    @param onSuccess Success callback with a stream that contains media stream.
  */
  void Subscribe(
      std::shared_ptr<RemoteStream> stream,
      const SubscribeOptions& options,
      std::function<void(std::shared_ptr<RemoteStream> stream)> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  /**
    @brief Un-publish the stream from the current room.
    @param stream The stream to be unpublished.
  */
  void Unpublish(
      std::shared_ptr<LocalStream> stream,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  /**
    @brief Un-subscribe the stream from the current room.
    @param stream The stream to be unsubscribed.
  */
  void Unsubscribe(
      std::shared_ptr<RemoteStream> stream,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  /**
    @brief Send messsage to all participants in the conference.
    @param message The message to be sent.
  */
  void Send(
      const std::string& message,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  /**
    @brief Send messsage to all participants in the conference.
    @param message The message to be sent.
    @param receiver Receiver's user ID.
  */
  void Send(
      const std::string& message,
      const std::string& receiver,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  /**
    @brief Continue to transmit specified stream's audio data.
    @detail If |stream| is a remote stream, MCU will continue to send audio data
    to client. If |stream| is a local stream, client will continue to send audio
    data to MCU. This method is expected to be called after |DisableAudio|.
  */
  void PlayAudio(
      std::shared_ptr<Stream> stream,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  /**
    @brief Stop transmitting specified stream's audio data.
    @detail If |stream| is a remote stream, MCU will stop sending audio data to
    client. If |stream| is a local stream, client will stop sending audio data
    to MCU.
  */
  void PauseAudio(
      std::shared_ptr<Stream> stream,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  /**
    @brief Continue to transmit specified stream's video data.
    @detail If |stream| is a remote stream, MCU will continue to send video data
    to client. If |stream| is a local stream, client will continue to send video
    data to MCU. This method is expected to be called after |DisableVideo|.
  */
  void PlayVideo(
      std::shared_ptr<Stream> stream,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  /**
    @brief Stop transmitting specified stream's video data.
    @detail If |stream| is a remote stream, MCU will stop sending video data to
    client. If |stream| is a local stream, client will stop sending video data
    to MCU.
  */
  void PauseVideo(
      std::shared_ptr<Stream> stream,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  /**
    @brief Leave current conference.
  */
  void Leave(
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);

 protected:
  virtual void OnStreamAdded(std::shared_ptr<sio::message> stream) override;
  virtual void OnCustomMessage(std::string& from,
                               std::string& message) override;
  virtual void OnSignalingMessage(std::shared_ptr<sio::message> stream) override;
  virtual void OnUserJoined(
      std::shared_ptr<const conference::User> user) override;
  virtual void OnUserLeft(
      std::shared_ptr<const conference::User> user) override;
  virtual void OnStreamRemoved(std::shared_ptr<sio::message> stream) override;
  virtual void OnServerDisconnected() override;
  virtual void OnStreamId(const std::string& id,
                          const std::string& publish_stream_label) override;

 private:
  bool CheckNullPointer(
      uintptr_t pointer,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  void TriggerOnStreamAdded(std::shared_ptr<sio::message> stream_info);
  PeerConnectionChannelConfiguration GetPeerConnectionChannelConfiguration()
      const;
  // Get the |ConferencePeerConnectionChannel| instance associated with specific
  // |stream|. Return |nullptr| if not found.
  std::shared_ptr<ConferencePeerConnectionChannel>
  GetConferencePeerConnectionChannel(std::shared_ptr<Stream> stream) const;
  std::shared_ptr<ConferencePeerConnectionChannel>
  GetConferencePeerConnectionChannel(const std::string& stream_id) const;
  void TriggerOnStreamRemoved(std::shared_ptr<sio::message> stream_info);
  conference::User ParseUser(std::shared_ptr<sio::message> user_info) const;

  enum StreamType: int;

  ConferenceClientConfiguration configuration_;
  std::shared_ptr<ConferenceSocketSignalingChannel> signaling_channel_;
  // Key woogeen::Stream's ID, value is MediaStream's label
  std::unordered_map<std::string, std::string> publish_id_label_map_;
  // Key is woogeen::Stream's ID.
  std::unordered_map<std::string,
                     std::shared_ptr<ConferencePeerConnectionChannel>>
      publish_pcs_;
  // Key is woogeen::Stream's ID
  std::unordered_map<std::string,
                     std::shared_ptr<ConferencePeerConnectionChannel>>
      subscribe_pcs_;
  // Key is woogeen::Stream's ID
  std::unordered_map<std::string, std::shared_ptr<RemoteStream>>
      added_streams_;
  std::unordered_map<std::string, StreamType> added_stream_type_;
  std::vector<std::reference_wrapper<ConferenceClientObserver>> observers_;
};
}
}

#endif  // WOOGEEN_CONFERENCE_CONFERENCECLIENT_H_
