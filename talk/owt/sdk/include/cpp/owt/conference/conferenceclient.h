// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_CONFERENCE_CONFERENCECLIENT_H_
#define OWT_CONFERENCE_CONFERENCECLIENT_H_
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <set>
#include "owt/base/commontypes.h"
#include "owt/base/clientconfiguration.h"
#include "owt/base/connectionstats.h"
#include "owt/base/macros.h"
#include "owt/base/options.h"
#include "owt/base/stream.h"
#include "owt/base/exception.h"
#include "owt/conference/conferencepublication.h"
#include "owt/conference/conferencesubscription.h"
#include "owt/conference/streamupdateobserver.h"
#include "owt/conference/subscribeoptions.h"
#ifdef OWT_ENABLE_QUIC
#include "owt/quic/web_transport_stream_interface.h"
#endif
namespace sio{
  class message;
}
namespace webrtc{
  class StatsReport;
}
namespace owt {
namespace base {
  struct PeerConnectionChannelConfiguration;
}
}
namespace owt {
namespace conference {
class Participant;
using namespace owt::base;

#ifdef OWT_ENABLE_QUIC
using namespace owt::quic;
typedef std::string cert_fingerprint_t;
#endif

/**
  @brief Configuration for creating a ConferenceClient
  @details This configuration is used while creating ConferenceClient.
  Changing this configuration does NOT impact ConferenceClient already
  created.
*/
struct ConferenceClientConfiguration : public ClientConfiguration {
#ifdef OWT_ENABLE_QUIC
 public:
  // This function sets trusted server certificate fingerprints for
  // QUIC connections. If fingerprints is empty, will use webpki for certificate
  // verification. Fingerprint should be a string of format "xx:xx:xx..." which
  // contains SHA-256 of the certificate fingerprint. See more details of the MCU
  // document for getting this value.
  std::vector<cert_fingerprint_t> trusted_quic_certificate_fingerprints;
#endif
};

class RemoteMixedStream;
class ConferencePeerConnectionChannel;
#ifdef OWT_ENABLE_QUIC
class ConferenceWebTransportChannel;
#endif
class ConferenceSocketSignalingChannel;
/**
  @brief Observer interface for participant
  @details Provides interface for receiving events with regard to the associated
  participant.
*/
class ParticipantObserver {
  public:
   virtual ~ParticipantObserver() = default;
   /**
     @brief Participant leave event callback.
   */
   virtual void OnLeft() {}
};
/// Participant represents one conference client in a conference room.
class Participant {
  friend class ConferenceInfo;
  public:
    Participant(std::string id, std::string role, std::string user_id)
      : id_(id)
      , role_(role)
      , user_id_(user_id) {}
    virtual ~Participant() {}
    /// Add an observer for pariticipant.
    void AddObserver(ParticipantObserver& observer);
    /// Remove an observer for participant.
    void RemoveObserver(ParticipantObserver& observer);
    /// Get the participant's ID.
    std::string Id() const { return id_; }
    /// Get the participant's role.
    std::string Role() const {return role_; }
    /// Get the participant's user id.
    std::string UserId() const { return user_id_; }
  protected:
    /// Set the participant's ID.
    void Id(std::string id) { id_ = id; }
    /// Set the participant's role.
    void Role(std::string role) { role_ = role; }
    /// Set the participant's user id.
    void UserId(std::string user_id) { user_id_ = user_id; }
    /// Trigger participant leave event on observers.
    void TriggerOnParticipantLeft();
  private:
    std::string id_;        /// Unique id assigned by MCU portal
    std::string role_;      /// Role of the participant
    std::string user_id_;   /// User account system assigned user id.
    mutable std::mutex observer_mutex_;
    std::vector<std::reference_wrapper<ParticipantObserver>> observers_;
};
/**
  @brief Information about the conference.
  @details This information contains current details of the conference.
*/
class ConferenceInfo {
  friend class ConferenceClient;
  public:
    ConferenceInfo() {}
    virtual ~ConferenceInfo() {}
    /// Current remote streams in the conference.
    std::vector<std::shared_ptr<RemoteStream>> RemoteStreams() const {
      return remote_streams_;
    }
    /// Current participant list in the conference.
    std::vector<std::shared_ptr<Participant>> Participants() const {
      return participants_;
    }
    /// Conference ID.
    std::string Id() const { return id_; }
    /// The participant info of current conference client.
    std::shared_ptr<Participant> Self() const {
      return self_;
    }
   protected:
    // Add participant.
    void AddParticipant(std::shared_ptr<Participant> participant);
    // Remove participant.
    void RemoveParticipantById(const std::string& id);
    // Add remote stream.
    void AddOrUpdateStream(std::shared_ptr<RemoteStream> remote_stream, bool &updated);
    // Remove remote stream.
    void RemoveStreamById(const std::string& stream_id);
    // Trigger participant left event.
    void TriggerOnParticipantLeft(const std::string& participant_id);
    // Trigger stream ended event.
    void TriggerOnStreamEnded(const std::string& stream_id);
    // Trigger stream updated event.
    void TriggerOnStreamUpdated(const std::string& stream_id);
    // Trigger stream mute/unmute events
    void TriggerOnStreamMuteOrUnmute(const std::string& stream_id, owt::base::TrackKind track_kind, bool muted);
  private:
    bool ParticipantPresent(const std::string& participant_id);
    bool RemoteStreamPresent(const std::string& stream_id);
    std::string id_;                           // Unique id that identifies the conference.
    mutable std::mutex participants_mutex_;
    std::vector<std::shared_ptr<Participant>> participants_;    // Participants in the conference
    mutable std::mutex remote_streams_mutex_;
    std::vector<std::shared_ptr<RemoteStream>> remote_streams_; // Remote streams in the conference.
    std::shared_ptr<Participant> self_;                           // Self participant in the conference.
};
/** @cond */
class ConferenceSocketSignalingChannelObserver {
 public:
  virtual ~ConferenceSocketSignalingChannelObserver(){}
  virtual void OnUserJoined(std::shared_ptr<sio::message> user) = 0;
  virtual void OnUserLeft(std::shared_ptr<sio::message> user) = 0;
  virtual void OnStreamAdded(std::shared_ptr<sio::message> stream) = 0;
  virtual void OnStreamRemoved(std::shared_ptr<sio::message> stream) = 0;
  virtual void OnStreamUpdated(std::shared_ptr<sio::message> stream) = 0;
  virtual void OnServerDisconnected() = 0;
  virtual void OnCustomMessage(std::string& from, std::string& message, std::string& to) = 0;
  virtual void OnSignalingMessage(std::shared_ptr<sio::message> message) = 0;
  virtual void OnStreamError(std::shared_ptr<sio::message> stream) = 0;
  // Notify the ID for a published/subscribed stream.
  virtual void OnStreamId(const std::string& id, const std::string& label) = 0;
  virtual void OnSubscriptionId(const std::string& subscription_id,
                                const std::string& stream_id) = 0;
};
// ConferencePeerConnectionChannel callback interface.
// Usually, ConferenceClient should implement these methods and notify
// application.
class ConferencePeerConnectionChannelObserver {
 public:
  virtual ~ConferencePeerConnectionChannelObserver(){}
  // Triggered when an unrecoverable error happened. Error may reported by MCU
  // or detected by client. Currently, only errors from MCU are handled.
  virtual void OnStreamError(
      std::shared_ptr<Stream> stream,
      std::shared_ptr<const Exception> exception) = 0;
};
#ifdef OWT_ENABLE_QUIC
// The visitor interface for QuicTransportClientInterface
class ConferenceWebTransportChannelObserver {
 public:
  virtual ~ConferenceWebTransportChannelObserver() = default;
  // Called when connected to a server.
  virtual void OnConnected() = 0;
  // Called when a connection is failed.
  virtual void OnConnectionFailed() = 0;
  // Called when an incoming stream is received by QUIC channel
  virtual void OnIncomingStream(
      const std::string& session_id,
      owt::quic::WebTransportStreamInterface* stream) = 0;
};
#endif
/** @endcond */
/// Observer for OWTConferenceClient.
class ConferenceClientObserver {
 public:
  /**
    @brief Triggers when a stream is added.
    @param stream The stream which is added.
  */
  virtual void OnStreamAdded(
      std::shared_ptr<RemoteStream> stream){}
  /**
    @brief Triggers when a mixed stream is added.
    @param stream The stream which is added.
  */
  virtual void OnStreamAdded(
      std::shared_ptr<RemoteMixedStream> stream){}
  /**
    @brief Triggers when a message is received.
    @param message Message received.
    @param sender_id Sender's ID.
    @param to "all" if it is a broadcast message. "me"
    if it is sent only to current conference client.
  */
  virtual void OnMessageReceived(std::string& message,
                                 std::string& sender_id,
                                 std::string& to){}
  /**
    @brief Triggers when a participant joined conference.
    @param user The user joined.
  */
  virtual void OnParticipantJoined(std::shared_ptr<Participant>){}
  /**
    @brief Triggers when server is disconnected.
  */
  virtual void OnServerDisconnected(){}
};

/// An asynchronous class for app to communicate with a conference in MCU.
class ConferenceClient final
    : ConferenceSocketSignalingChannelObserver,
      ConferencePeerConnectionChannelObserver,
#ifdef OWT_ENABLE_QUIC
      owt::conference::ConferenceWebTransportChannelObserver,
#endif
      public std::enable_shared_from_this<ConferenceClient> {
  friend class ConferencePublication;
  friend class ConferenceSubscription;
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
    current conference information.
  */
  void Join(
      const std::string& token,
      std::function<void(std::shared_ptr<ConferenceInfo>)> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  /**
    @brief Leave current conference.
  */
  void Leave(
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  /**
    @brief Publish the stream to the current room.
    @param stream The stream to be published.
  */
  void Publish(
      std::shared_ptr<LocalStream> stream,
      std::function<void(std::shared_ptr<ConferencePublication>)> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  /**
    @brief Publish the stream to the current room.
    @param stream The stream to be published.
    @param options Options for publishing the stream.
  */
  void Publish(
      std::shared_ptr<LocalStream> stream,
      const PublishOptions& options,
      std::function<void(std::shared_ptr<ConferencePublication>)> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  /**
    @brief Subscribe a stream from the current room.
    @param stream The remote stream to be subscribed.
    @param onSuccess Success callback with a stream that contains media stream.
  */
  void Subscribe(
      std::shared_ptr<RemoteStream> stream,
      std::function<void(std::shared_ptr<ConferenceSubscription>)> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  /**
    @brief Subscribe a stream from the current room.
    @param stream The remote stream to be subscribed.
    @param options Options for subscribing the stream.
    @param onSuccess Success callback with a stream that contains media stream.
  */
  void Subscribe(
      std::shared_ptr<RemoteStream> stream,
      const SubscribeOptions& options,
      std::function<void(std::shared_ptr<ConferenceSubscription>)> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  /**
    @brief Send messsage to all participants in the conference.
    @param message The message to be sent.
  */
  void Send(
      const std::string& message,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  /**
    @brief Send messsage to all participants in the conference.
    @param message The message to be sent.
    @param receiver Receiver's user ID.
  */
  void Send(
      const std::string& message,
      const std::string& receiver,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
#ifdef OWT_ENABLE_QUIC
  /**
   @brief Creates a LocalStream for WebTransport.
   @details Please be noted this can only be called when
   client is connected to MCU(successfully joined).
   @param onSuccess Success callback with a LocalStream on which
   you can get a QuicStream.
  */
  void CreateSendStream(
      std::function<void(std::shared_ptr<owt::base::LocalStream>)> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
#endif
 protected:
  ConferenceClient(const ConferenceClientConfiguration& configuration);
  // Implementing ConferenceSocketSignalingChannelObserver.
  virtual void OnStreamAdded(std::shared_ptr<sio::message> stream) override;
  virtual void OnCustomMessage(std::string& from,
                               std::string& message,
                               std::string& to) override;
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
  virtual void OnSubscriptionId(const std::string& subscription_id,
                                const std::string& stream_id) override;
  // Implementing ConferencePeerConnectionChannelObserver.
  virtual void OnStreamError(
      std::shared_ptr<Stream> stream,
      std::shared_ptr<const Exception> exception) override;
  // Provide access for Publication and Subscription instances.
  /**
    @brief Un-publish the stream from the current room.
    @param stream The stream to be unpublished.
  */
  void UnPublish(
      const std::string& session_id,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  /**
    @brief Un-subscribe the stream from the current room.
    @param stream The stream to be unsubscribed.
  */
  void UnSubscribe(
      const std::string& session_id,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  /**
   @brief Update a subscription.
  */
  void UpdateSubscription(
      const std::string& session_id,
      const std::string& stream_id,
      const SubscriptionUpdateOptions& option,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  /**
    @brief Deprecated. Get a stream's connection statistics
  */
  OWT_DEPRECATED void GetConnectionStats(
      const std::string& session_id,
      std::function<void(std::shared_ptr<ConnectionStats>)> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  /**
   @brief Get a stream's connection statistics
  */
  void GetConnectionStats(
      const std::string& session_id,
      std::function<void(std::shared_ptr<RTCStatsReport>)> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  void GetStats(
      const std::string& session_id,
      std::function<void(
          const std::vector<const webrtc::StatsReport*>& reports)> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  /**
    @brief Mute a session's track specified by |track_kind|.
  */
  void Mute(
      const std::string& session_id,
      TrackKind track_kind,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  /**
    @brief Unmute a session's track specified by |track_kind|.
  */
  void Unmute(
      const std::string& session_id,
      TrackKind track_kind,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
 private:
#ifdef OWT_ENABLE_QUIC
  // Overrides WebTransportChannelObserver
  void OnConnected();
  void OnConnectionFailed();
  void OnIncomingStream(const std::string& session_id,
                        owt::quic::WebTransportStreamInterface* stream);
#endif

  /// Return true if |pointer| is not a null pointer, else return false and
  /// trigger |on_failure| with |failure_message|.
  bool CheckNullPointer(
      uintptr_t pointer,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  bool CheckNullPointer(
      uintptr_t pointer,
      const std::string& failure_message,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  /// Return true if signaling channel is connected, else return false and
  /// trigger |on_failure|
  bool CheckSignalingChannelOnline(
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  PeerConnectionChannelConfiguration GetPeerConnectionChannelConfiguration()
      const;
  // Get the |ConferencePeerConnectionChannel| instance associated with specific
  // |session_id|. Return |nullptr| if not found.
  std::shared_ptr<ConferencePeerConnectionChannel>
  GetConferencePeerConnectionChannel(const std::string& session_id) const;
  void TriggerOnUserJoined(std::shared_ptr<sio::message> user_info, bool joining = false);
  void TriggerOnUserLeft(std::shared_ptr<sio::message> user_info);
  void TriggerOnStreamAdded(std::shared_ptr<sio::message> stream_info, bool joining = false);
  void TriggerOnStreamRemoved(std::shared_ptr<sio::message> stream_info);
  void TriggerOnStreamUpdated(std::shared_ptr<sio::message> stream_info);
  void TriggerOnStreamError(std::shared_ptr<Stream> stream,
                            std::shared_ptr<const Exception> exception);
#ifdef OWT_ENABLE_QUIC
  void TriggerOnIncomingStream(const std::string& session_id,
                               owt::quic::WebTransportStreamInterface* stream);
#endif
  // Return true if |user_info| is correct, and |*participant| points to the participant
  // object
  bool ParseUser(std::shared_ptr<sio::message> user_info, Participant** participant) const;
  void ParseStreamInfo(std::shared_ptr<sio::message> stream_info, bool joining = false);
  std::unordered_map<std::string, std::string> AttributesFromStreamInfo(
      std::shared_ptr<sio::message> stream_info);
  std::function<void()> RunInEventQueue(std::function<void()> func);
  // Check if all characters are base 64 allowed or '='.
  bool IsBase64EncodedString(const std::string str) const;
  /// Add an observer for conferenc client.
  void AddStreamUpdateObserver(ConferenceStreamUpdateObserver& observer);
  /// Remove an object from conference client.
  void RemoveStreamUpdateObserver(ConferenceStreamUpdateObserver& observer);
#ifdef OWT_ENABLE_QUIC
  void InitializeQuicClientIfSupported(const std::string& token_base64);
  std::string WebTransportId();
  bool ParseWebTransportToken();
#endif
  enum StreamType: int;
  ConferenceClientConfiguration configuration_;
  // Queue for callbacks and events. Shared among ConferenceClient and all of
  // it's ConferencePeerConnectionChannels or ConferenceWebTransportChannels
  std::shared_ptr<rtc::TaskQueue> event_queue_;
  std::shared_ptr<ConferenceSocketSignalingChannel> signaling_channel_;
  std::mutex observer_mutex_;
  bool signaling_channel_connected_;
  // Key publish(session) ID from server, value is MediaStream's label
  std::unordered_map<std::string, std::string> publish_id_label_map_;
  // Store the peer connection channels created.
  std::vector<std::shared_ptr<ConferencePeerConnectionChannel>>
      publish_pcs_;
  mutable std::mutex publish_pcs_mutex_;
  // Key is subcription ID from server.
  std::vector<std::shared_ptr<ConferencePeerConnectionChannel>>
      subscribe_pcs_;
  // Key is subscription ID, value is streamID.
  std::unordered_map<std::string, std::string> subscribe_id_label_map_;
  mutable std::mutex subscribe_pcs_mutex_;
  // Key is the stream ID(publication ID or mixed stream ID).
  std::unordered_map<std::string, std::shared_ptr<RemoteStream>>
      added_streams_;
  std::unordered_map<std::string, StreamType> added_stream_type_;
  mutable std::mutex conference_info_mutex_;
  // Store current conference info.
  std::shared_ptr<ConferenceInfo> current_conference_info_;
  // Capturing observer in |event_queue_| is not 100% safe although above queue
  // is excepted to be ended after ConferenceClient is destroyed.
  std::vector<std::reference_wrapper<ConferenceClientObserver>> observers_;
  mutable std::mutex stream_update_observer_mutex_;
  std::vector <std::reference_wrapper<ConferenceStreamUpdateObserver>> stream_update_observers_;
#ifdef OWT_ENABLE_QUIC
  // Each conference client will be associated with only one quic_transport_channel_ instance.
  std::shared_ptr<ConferenceWebTransportChannel> web_transport_channel_;
  std::atomic<bool> web_transport_channel_connected_;
  std::string webtransport_id_;  // transportId used for publish/subscribe.
  // base64 encoded webTransportToken recevied after joining. key is
  // session_id(publication_id)
  std::string webtransport_token_;
  std::map<std::string, std::shared_ptr<ConferencePublication>> quic_publications_;
  mutable std::mutex quic_publications_mutex_;
  // key is session_id(subscription_id)
  std::map<std::string, std::shared_ptr<ConferenceSubscription>> quic_subscriptions_;
  mutable std::mutex quic_subscriptions_mutex_;
  std::unordered_map<std::string, WebTransportStreamInterface*>
      pending_incoming_streams_;
  mutable std::mutex pending_quic_streams_mutex_;
#endif
};
}
}
#endif  // OWT_CONFERENCE_CONFERENCECLIENT_H_
