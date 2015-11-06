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

#include <memory>
#include <unordered_map>
#include <vector>
#include "clientconfiguration.h"
#include "conferenceexception.h"
#include "conferencesocketsignalingchannel.h"
#include "stream.h"

namespace woogeen {

struct ConferenceClientConfiguration : ClientConfiguration {};

class RemoteMixedStream;
class ConferencePeerConnectionChannel;
struct PeerConnectionChannelConfiguration;

class ConferenceClientObserver {
 public:
  // Triggered when a new stream is added.
  virtual void OnStreamAdded(
      std::shared_ptr<woogeen::RemoteCameraStream> stream){};
  virtual void OnStreamAdded(
      std::shared_ptr<woogeen::RemoteScreenStream> stream){};
  virtual void OnStreamAdded(
      std::shared_ptr<woogeen::RemoteMixedStream> stream){};
  // Triggered when a remote stream is removed.
  virtual void OnStreamRemoved(
      std::shared_ptr<woogeen::RemoteCameraStream> stream){};
  virtual void OnStreamRemoved(
      std::shared_ptr<woogeen::RemoteScreenStream> stream){};
  virtual void OnStreamRemoved(
      std::shared_ptr<woogeen::RemoteMixedStream> stream){};
  // Triggered when received a message.
  virtual void OnMessageReceived(std::string& sender_id,
                                 std::string& message){};
  virtual void OnUserJoined(std::shared_ptr<const conference::User>){};
  virtual void OnUserLeft(std::shared_ptr<const conference::User>){};
  // Triggered when server is disconnected.
  virtual void OnServerDisconnected(){};
  // TODO(jianjun): add other events.
};

class ConferenceClient final : ConferenceSocketSignalingChannelObserver {
 public:
  ConferenceClient(
      ConferenceClientConfiguration& configuration,
      std::shared_ptr<ConferenceSocketSignalingChannel> signaling_channel);
  // Add an observer for conferenc client.
  void AddObserver(std::shared_ptr<ConferenceClientObserver> observer);
  // Remove an object from conference client.
  void RemoveObserver(std::shared_ptr<ConferenceClientObserver> observer);
  // Join a conference.
  void Join(
      const std::string& token,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  // Publish a stream to the conference.
  void Publish(
      std::shared_ptr<LocalStream> stream,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  // Subscribe a stream from the conference.
  void Subscribe(
      std::shared_ptr<RemoteStream> stream,
      std::function<void(std::shared_ptr<RemoteStream> stream)> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  // Unpublish a stream to the conference.
  void Unpublish(
      std::shared_ptr<LocalStream> stream,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  // Unsubscribe a stream from the conference.
  void Unsubscribe(
      std::shared_ptr<RemoteStream> stream,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  // Send a message to all participants in the conference.
  void Send(
      const std::string& message,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  // Send a message to a specified participant.
  void Send(
      const std::string& message,
      const std::string& receiver,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  // Continue to transmit specified stream's audio data.
  // If |stream| is a remote stream, MCU will continue to send audio data to
  // client. If |stream| is a local stream, client will continue to send audio
  // data to MCU. This method is expected to be called after |DisableAudio|.
  void PlayAudio(
      std::shared_ptr<Stream> stream,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  // Stop transmitting specified stream's audio data.
  // If |stream| is a remote stream, MCU will stop sending audio data to client.
  // If |stream| is a local stream, client will stop sending audio data to MCU.
  void PauseAudio(
      std::shared_ptr<Stream> stream,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  // Continue to transmit specified stream's video data.
  // If |stream| is a remote stream, MCU will continue to send video data to
  // client. If |stream| is a local stream, client will continue to send video
  // data to MCU. This method is expected to be called after |DisableVideo|.
  void PlayVideo(
      std::shared_ptr<Stream> stream,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  // Stop transmitting specified stream's video data.
  // If |stream| is a remote stream, MCU will stop sending video data to client.
  // If |stream| is a local stream, client will stop sending video data to MCU.
  void PauseVideo(
      std::shared_ptr<Stream> stream,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  // Leave this conference.
  void Leave(
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);

 protected:
  virtual void OnStreamAdded(sio::message::ptr stream) override;
  virtual void OnCustomMessage(std::string& from,
                               std::string& message) override;
  virtual void OnSignalingMessage(sio::message::ptr stream) override;
  virtual void OnUserJoined(
      std::shared_ptr<const conference::User> user) override;
  virtual void OnUserLeft(
      std::shared_ptr<const conference::User> user) override;
  virtual void OnStreamRemoved(sio::message::ptr stream) override;
  virtual void OnServerDisconnected() override;
  virtual void OnStreamId(const std::string& id,
                          const std::string& publish_stream_label) override;

 private:
  bool CheckNullPointer(
      uintptr_t pointer,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  void TriggerOnStreamAdded(sio::message::ptr stream_info);
  PeerConnectionChannelConfiguration GetPeerConnectionChannelConfiguration()
      const;
  // Get the |ConferencePeerConnectionChannel| instance associated with specific
  // |stream|. Return |nullptr| if not found.
  std::shared_ptr<ConferencePeerConnectionChannel>
  GetConferencePeerConnectionChannel(std::shared_ptr<Stream> stream) const;
  std::shared_ptr<ConferencePeerConnectionChannel>
  GetConferencePeerConnectionChannel(const std::string& stream_id) const;
  void TriggerOnStreamRemoved(sio::message::ptr stream_info);
  conference::User ParseUser(sio::message::ptr user_info) const;

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
  std::unordered_map<std::string, std::shared_ptr<woogeen::RemoteStream>>
      added_streams_;
  std::unordered_map<std::string, StreamType> added_stream_type_;
  std::vector<std::shared_ptr<ConferenceClientObserver>> observers_;
};
}

#endif  // WOOGEEN_CONFERENCE_CONFERENCECLIENT_H_
