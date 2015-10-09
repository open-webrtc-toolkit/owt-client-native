/*
 * Intel License
 */

#ifndef WOOGEEN_CONFERENCE_CONFERENCECLIENT_H_
#define WOOGEEN_CONFERENCE_CONFERENCECLIENT_H_

#include <memory>
#include <unordered_map>
#include <vector>
#include "talk/woogeen/sdk/base/clientconfiguration.h"
#include "talk/woogeen/sdk/conference/conferenceexception.h"
#include "talk/woogeen/sdk/conference/conferencesignalingchannelinterface.h"
#include "talk/woogeen/sdk/conference/conferencepeerconnectionchannel.h"
#include "talk/woogeen/sdk/conference/remotemixedstream.h"

namespace woogeen {

struct ConferenceClientConfiguration : ClientConfiguration {};

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

class ConferenceClient final : ConferenceSignalingChannelObserver {
 public:
  ConferenceClient(
      ConferenceClientConfiguration& configuration,
      std::shared_ptr<ConferenceSignalingChannelInterface> signaling_channel);
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
  virtual void OnStreamAdded(Json::Value stream) override;
  virtual void OnCustomMessage(std::string& from,
                               std::string& message) override;
  virtual void OnUserJoined(
      std::shared_ptr<const conference::User> user) override;
  virtual void OnUserLeft(
      std::shared_ptr<const conference::User> user) override;
  virtual void OnStreamRemoved(Json::Value stream) override;
  virtual void OnServerDisconnected() override;

 private:
  bool CheckNullPointer(
      uintptr_t pointer,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  void TriggerOnStreamAdded(const Json::Value& stream_info);
  PeerConnectionChannelConfiguration GetPeerConnectionChannelConfiguration()
      const;
  // Get the |ConferencePeerConnectionChannel| instance associated with specific
  // |stream|. Return |nullptr| if not found.
  std::shared_ptr<ConferencePeerConnectionChannel>
  GetConferencePeerConnectionChannel(std::shared_ptr<Stream> stream) const;
  void TriggerOnStreamRemoved(const Json::Value& stream_info);

  ConferenceClientConfiguration configuration_;
  std::shared_ptr<ConferenceSignalingChannelInterface> signaling_channel_;
  std::unordered_map<std::string,
                     std::shared_ptr<ConferencePeerConnectionChannel>>
      publish_pcs_;
  std::unordered_map<std::string,
                     std::shared_ptr<ConferencePeerConnectionChannel>>
      subscribe_pcs_;
  std::unordered_map<std::string, std::shared_ptr<woogeen::RemoteStream>>
      added_streams_;
  std::vector<std::shared_ptr<ConferenceClientObserver>> observers_;
};
}

#endif  // WOOGEEN_CONFERENCE_CONFERENCECLIENT_H_
