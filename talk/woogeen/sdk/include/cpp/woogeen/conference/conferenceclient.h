/*
 * Copyright © 2016 Intel Corporation. All Rights Reserved.
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
#include <mutex>
#include <unordered_map>
#include <vector>

#include "woogeen/base/clientconfiguration.h"
#include "woogeen/base/connectionstats.h"
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

namespace webrtc {
  class CriticalSectionWrapper;
}

namespace rtc {
  class TaskQueue;
}

namespace woogeen {
namespace conference {

using namespace woogeen::base;

/** @cond */
/**
  @brief Publish session identity
  @details This structure is used to associate one publish session ID with
  certain local stream label. Multiple publish session ID can map to the same
  local stream label(ID).
  */
class ConferenceLocalPublishSession {
public:
  ConferenceLocalPublishSession(std::string session_id, std::string label)
    :session_id_(session_id),local_stream_id_(label) {}

  std::string SessionId() { return session_id_; }

  std::string LocalStreamId() { return local_stream_id_; }
private:
  std::string session_id_;
  std::string local_stream_id_;
};

/**
  @brief Subcribe session identity
  @details This structure is used to associate one subcribe session ID with
  certain remote stream's label. Multiple subcribe session ID can map to the
  same remote stream label(ID).
  */
struct ConferenceRemoteSubcribeSession {
public:
  ConferenceRemoteSubcribeSession(std::string session_id, std::string label)
    :session_id_(session_id),remote_stream_id_(label) {}

  std::string SessionId() { return session_id_; }

  std::string RemoteStreamId() { return remote_stream_id_; }
private:
  std::string session_id_;
  std::string remote_stream_id_;
};
/** @endcond */

/**
  @brief Configuration for creating a ConferenceClient
  @details This configuration is used while creating ConferenceClient.
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
  virtual ~ConferenceSocketSignalingChannelObserver(){};
  virtual void OnUserJoined(std::shared_ptr<sio::message> user) = 0;
  virtual void OnUserLeft(std::shared_ptr<sio::message> user) = 0;
  virtual void OnStreamAdded(std::shared_ptr<sio::message> stream) = 0;
  virtual void OnStreamRemoved(std::shared_ptr<sio::message> stream) = 0;
  virtual void OnStreamUpdated(std::shared_ptr<sio::message> stream) = 0;
  virtual void OnServerDisconnected() = 0;
  virtual void OnCustomMessage(std::string& from, std::string& message) = 0;
  virtual void OnSignalingMessage(std::shared_ptr<sio::message> message) = 0;
  virtual void OnStreamError(std::shared_ptr<sio::message> stream) = 0;
  // Notify the ID for a published stream.
  virtual void OnStreamId(const std::string& id, const std::string& label) = 0;
  // Notify the ID for a subscribed stream.
  virtual void OnRemoteStreamId(const std::string& id, const std::string& label) = 0;
};

// ConferencePeerConnectionChannel callback interface.
// Usually, ConferenceClient should implement these methods and notify
// application.
class ConferencePeerConnectionChannelObserver {
 public:
  virtual ~ConferencePeerConnectionChannelObserver(){};
  // Triggered when an unrecoverable error happened. Error may reported by MCU
  // or detected by client. Currently, only errors from MCU are handled.
  virtual void OnStreamError(
      std::shared_ptr<Stream> stream,
      std::shared_ptr<const ConferenceException> exception) = 0;
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
    @brief Triggers when an error happened on a stream.
    @details This event only triggered for a stream that is being published or
    subscribed. SDK will not try to recovery the certain stream when this event
    is triggered. If you still need this stream, please re-publish or
    re-subscribe.
    @param stream The stream which is corrupted. It might be a LocalStream or
    RemoteStream.
    @param exception The exception happened. Currently, exceptions are reported
    by MCU.
  */
  virtual void OnStreamError(
      std::shared_ptr<Stream> stream,
      std::unique_ptr<ConferenceException> exception){};
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
  /**
    @brief Triggers when server is disconnected.
  */
  virtual void OnServerDisconnected(){};
};

/// Options for publishing a stream.
struct PublishOptions {
  /**
   @brief Max outgoing audio bandwidth, unit: kbps.
   @details Please be noticed different codecs may support different bitrate
   ranges. If you set a bandwidth limitation which is not supported by selected
   codec, connection will fail. If it is set to 0, associated ConferenceClient's
   max audio bandwidth will be used.
   */
  int max_audio_bandwidth;
  /**
   @brief Max outgoing video bandwidth, unit: kbps.
   @details Please be noticed different codecs may support different bitrate
   ranges. If you set a bandwidth limitation which is not supported by selected
   codec, connection will fail. If it is set to 0, associated ConferenceClient's
   max video bandwidth will be used.
   */
  int max_video_bandwidth;
  /**
   @brief Construct PublishOptions with default values.
   @details Default values for max_audio_bandwidth and max_video_bandwidth are 0.
   */
  explicit PublishOptions() : max_audio_bandwidth(0), max_video_bandwidth(0) {}
};

/// An asynchronous class for app to communicate with a conference in MCU.
class ConferenceClient final
    : ConferenceSocketSignalingChannelObserver,
      ConferencePeerConnectionChannelObserver,
      public std::enable_shared_from_this<ConferenceClient> {
 public:
  /**
    @brief Create a ConferenceClient instance with specific configuration
    @param configuration Configuration for creating the ConferenceClient.
  */
  static std::shared_ptr<ConferenceClient> Create(
      const ConferenceClientConfiguration& configuration);
  ~ConferenceClient();
  /// Add an observer for conferenc client.
  void AddObserver(ConferenceClientObserver& observer);
  /// Remove an object from conference client.
  void RemoveObserver(ConferenceClientObserver& observer);
  /**
    @brief Connect to the specified room to join a conference.
    @param token Includes the room info which is encrypted.
    @param on_success Join conference success, on_sucess will be executed with
    current user's information.
  */
  void Join(
      const std::string& token,
      std::function<void(std::shared_ptr<User>)> on_success,
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
    @brief Publish the stream to the current room.
    @param stream The stream to be published.
    @param options Options for publishing the stream.
  */
  void Publish(
      std::shared_ptr<LocalStream> stream,
      const PublishOptions& options,
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
    @brief Continue to send specified stream's audio data.
  */
  void PlayAudio(
      std::shared_ptr<LocalStream> stream,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  /**
    @brief Stop sending specified stream's audio data.
  */
  void PauseAudio(
      std::shared_ptr<LocalStream> stream,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  /**
    @brief Continue to send specified stream's video data.
  */
  void PlayVideo(
      std::shared_ptr<LocalStream> stream,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  /**
    @brief Stop sending specified stream's video data.
  */
  void PauseVideo(
      std::shared_ptr<LocalStream> stream,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  /**
    @brief Continue to receive specified stream's audio data.
    @details MCU will continue to send audio data to client even stream's audio
    track is disabled.
  */
  void PlayAudio(
      std::shared_ptr<RemoteStream> stream,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  /**
    @brief Stop receiving specified stream's audio data.
    @details MCU will stop sending audio data to client. It saves network traffic
    if audio track is disabled.
  */
  void PauseAudio(
      std::shared_ptr<RemoteStream> stream,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  /**
    @brief Continue to receive specified stream's video data.
    @details MCU will continue to send video data to client even stream's video
    track is disabled or there is no video sink associated with specific stream.
  */
  void PlayVideo(
      std::shared_ptr<RemoteStream> stream,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  /**
    @brief Stop receiving specified stream's video data.
    @details MCU will stop sending video data to client. If saves network traffic
    if video track is disabled.
    to MCU.
  */
  void PauseVideo(
      std::shared_ptr<RemoteStream> stream,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  /**
    @brief Leave current conference.
  */
  void Leave(
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);

  /**
    @brief Get a remote stream's region.
    @param stream Whose region to get.
    @param mixed_stream Mixed stream on which the region to get.
    @param on_success Success callback with region ID.
  */
  void GetRegion(
      std::shared_ptr<RemoteStream> stream,
      std::shared_ptr<RemoteMixedStream> mixed_stream,
      std::function<void(std::string)> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);

  /**
    @brief Set a remote stream's region.
    @param stream Whose region to set.
    @param mixed_stream Mixed stream on which the region to get.
    @param region_id Region ID to be set.
  */
  void SetRegion(
      std::shared_ptr<RemoteStream> stream,
      std::shared_ptr<RemoteMixedStream> mixed_stream,
      const std::string& region_id,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);

  /**
    @brief Mute a stream on conference server so server will actively drop
    video or(and) audio data from this stream depending on the |mute_video| and
    |mute_audio| parameters.
  */
  void Mute(
      std::shared_ptr<Stream> stream,
      bool mute_audio,
      bool mute_video,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);

  /**
    @brief Unmute a stream on conference server so server will resume receiving
    previously muted video or(and) audio data of this stream depending on the
    |unmute_video| and |unmute_audio| parameters.
  */
  void Unmute(
      std::shared_ptr<Stream> stream,
      bool unmute_audio,
      bool unmute_video,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);

  /**
    @brief Enable mixing of specified |stream| into a series of remote mixed
    streams. Providing empty remote stream list results in the |on_failure|
    callback to be invoked.
  */
  void Mix(
      std::shared_ptr<Stream> stream,
      std::vector<std::shared_ptr<RemoteMixedStream>> mixed_stream_list,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);

  /**
    @brief Disable mixing of specified |stream| into a series of remote mixed
    streams. Providing empty remote stream list results in the |on_failure|
    callback to be invoked.
  */
  void Unmix(
      std::shared_ptr<Stream> stream,
      std::vector<std::shared_ptr<RemoteMixedStream>> mixed_stream_list,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);

  /**
    @brief Get a stream's connection statistics
  */
  void GetConnectionStats(
      std::shared_ptr<Stream> stream,
      std::function<void(std::shared_ptr<ConnectionStats>)> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);

 protected:
  ConferenceClient(const ConferenceClientConfiguration& configuration);
  // Implementing ConferenceSocketSignalingChannelObserver.
  virtual void OnStreamAdded(std::shared_ptr<sio::message> stream) override;
  virtual void OnCustomMessage(std::string& from,
                               std::string& message) override;
  virtual void OnSignalingMessage(
      std::shared_ptr<sio::message> stream) override;
  virtual void OnUserJoined(std::shared_ptr<sio::message> user) override;
  virtual void OnUserLeft(std::shared_ptr<sio::message> user) override;
  virtual void OnStreamRemoved(std::shared_ptr<sio::message> stream) override;
  virtual void OnStreamUpdated(std::shared_ptr<sio::message> stream) override;
  virtual void OnStreamError(std::shared_ptr<sio::message> stream) override;
  virtual void OnServerDisconnected() override;
  virtual void OnStreamId(const std::string& id,
                          const std::string& publish_stream_label) override;
  virtual void OnRemoteStreamId(const std::string& id,
                                const std::string& subscribe_stream_label) override;
  // Implementing ConferencePeerConnectionChannelObserver.
  virtual void OnStreamError(
      std::shared_ptr<Stream> stream,
      std::shared_ptr<const ConferenceException> exception) override;

 private:
  /// Return true if |pointer| is not a null pointer, else return false and
  /// trigger |on_failure| with |failure_message|.
  bool CheckNullPointer(
      uintptr_t pointer,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  bool CheckNullPointer(
      uintptr_t pointer,
      const std::string& failure_message,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  /// Return true if signaling channel is connected, else return false and
  /// trigger |on_failure|
  bool CheckSignalingChannelOnline(
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  PeerConnectionChannelConfiguration GetPeerConnectionChannelConfiguration()
      const;
  // Get the |ConferencePeerConnectionChannel| instance associated with specific
  // |stream_id|. Return |nullptr| if not found.
  std::shared_ptr<ConferencePeerConnectionChannel>
  GetConferencePeerConnectionChannel(const std::string& stream_id) const;
  // Get the |ConferencePeerConnectionChannel| instance associated with specific
  // |stream|. Return |nullptr| if not found.
  std::shared_ptr<ConferencePeerConnectionChannel>
  GetConferencePeerConnectionChannel(std::shared_ptr<Stream> stream) const;
  // Get the |ConferencePeerConnectionChannel| instance associated with specific
  // |stream|. Return |nullptr| if not found.
  std::shared_ptr<ConferencePeerConnectionChannel>
  GetConferencePeerConnectionChannel(std::shared_ptr<LocalStream> stream) const;
  // Get the |ConferencePeerConnectionChannel| instance associated with specific
  // |stream|. Return |nullptr| if not found.
  std::shared_ptr<ConferencePeerConnectionChannel>
  GetConferencePeerConnectionChannel(
      std::shared_ptr<RemoteStream> stream) const;
  void TriggerOnUserJoined(std::shared_ptr<sio::message> user_info);
  void TriggerOnUserLeft(std::shared_ptr<sio::message> user_info);
  void TriggerOnStreamAdded(std::shared_ptr<sio::message> stream_info);
  void TriggerOnStreamRemoved(std::shared_ptr<sio::message> stream_info);
  void TriggerOnStreamUpdated(std::shared_ptr<sio::message> stream_info);
  void TriggerOnStreamError(std::shared_ptr<Stream> stream,
                            std::shared_ptr<const ConferenceException> exception);
  // Return true if |user_info| is correct, and |*user| points to the user
  // object
  bool ParseUser(std::shared_ptr<sio::message> user_info, User** user) const;
  std::unordered_map<std::string, std::string> AttributesFromStreamInfo(
      std::shared_ptr<sio::message> stream_info);
  std::function<void()> RunInEventQueue(std::function<void()> func);

  enum StreamType: int;

  ConferenceClientConfiguration configuration_;
  // Queue for callbacks and events. Shared among ConferenceClient and all of
  // it's ConferencePeerConnectionChannel.
  std::shared_ptr<rtc::TaskQueue> event_queue_;
  std::shared_ptr<ConferenceSocketSignalingChannel> signaling_channel_;
  std::mutex observer_mutex_;
  bool signaling_channel_connected_;
  // Key publish(session) ID from server, value is MediaStream's label
  std::unordered_map<std::string, std::string> publish_id_label_map_;
  // Key is publish(session) ID from server.
  std::unordered_map<std::string,
                     std::shared_ptr<ConferencePeerConnectionChannel>>
      publish_pcs_;
  mutable std::mutex publish_pcs_mutex_;
  // Key is subcription ID from server.
  std::unordered_map<std::string,
                     std::shared_ptr<ConferencePeerConnectionChannel>>
      subscribe_pcs_;
  // Key is subscription ID, value is streamID.
  std::unordered_map<std::string, std::string> subscribe_id_label_map_;
  mutable std::mutex subscribe_pcs_mutex_;
  // Key is the stream ID(publication ID or mixed stream ID).
  std::unordered_map<std::string, std::shared_ptr<RemoteStream>>
      added_streams_;
  std::unordered_map<std::string, StreamType> added_stream_type_;
  // Key is user's ID
  std::unordered_map<std::string, std::shared_ptr<User>> participants;
  // Capturing observer in |event_queue_| is not 100% safe although above queue
  // is excepted to be ended after ConferenceClient is destroyed.
  std::vector<std::reference_wrapper<ConferenceClientObserver>> observers_;
};
}
}

#endif  // WOOGEEN_CONFERENCE_CONFERENCECLIENT_H_
