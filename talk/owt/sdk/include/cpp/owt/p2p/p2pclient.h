// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_P2P_P2PCLIENT_H_
#define OWT_P2P_P2PCLIENT_H_
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <set>
#include "owt/base/commontypes.h"
#include "owt/base/connectionstats.h"
#include "owt/base/stream.h"
#include "owt/p2p/p2ppublication.h"
#include "owt/p2p/p2psignalingchannelinterface.h"
#include "owt/p2p/p2psignalingsenderinterface.h"
#include "owt/base/clientconfiguration.h"
#include "owt/base/globalconfiguration.h"
namespace rtc {
  class TaskQueue;
}
namespace owt {
namespace base {
  struct PeerConnectionChannelConfiguration;
}
namespace p2p{
/**
 @brief Configuration for P2PClient
 This configuration is used while creating P2PClient. Changing this
 configuration does NOT impact P2PClient already created.
*/
struct P2PClientConfiguration : owt::base::ClientConfiguration {
  std::vector<AudioEncodingParameters> audio_encodings;
  std::vector<VideoEncodingParameters> video_encodings;
};
class P2PPeerConnectionChannelObserverCppImpl;
class P2PPeerConnectionChannel;
/// Observer for P2PClient
class P2PClientObserver {
 public:
  /**
   @brief This function will be invoked when client is disconnected from
   signaling server.
   */
  virtual void OnServerDisconnected(){};
  /**
   @brief This function will be invoked when a remote user denied current user's
   invitation.
   @param remote_user_id Remote user’s ID
   */
  virtual void OnDenied(const std::string& remote_user_id){};
  /**
   @brief This function will be invoked when a chat is stopped. (This event
   haven't been implemented yet)
   @param remote_user_id Remote user’s ID
   */
  virtual void OnChatStopped(const std::string& remote_user_id){};
  /**
   @brief This function will be invoked when a chat is started. (This event
   haven't been implemented yet)
   @param remote_user_id Remote user’s ID
   */
  virtual void OnChatStarted(const std::string& remote_user_id){};
  /**
   @brief This function will be invoked when received data from a remote user.
   (This event haven't been implemented yet)
   @param remote_user_id Remote user’s ID
   @param message Message received
   */
  virtual void OnDataReceived(const std::string& remote_user_id,
                              const std::string message){};
  /**
   @brief This function will be invoked when a remote stream is available.
   @param stream The remote stream added.
   */
  virtual void OnStreamAdded(
      std::shared_ptr<owt::base::RemoteStream> stream){};
};
/// An async client for P2P WebRTC sessions
class P2PClient final
    : protected P2PSignalingSenderInterface,
      protected P2PSignalingChannelObserver,
      public std::enable_shared_from_this<P2PClient> {
  friend class P2PPublication;
  friend class P2PPeerConnectionChannelObserverCppImpl;
 public:
  /**
   @brief Init a P2PClient instance with speficied signaling channel.
   @param configuration Configuration for creating the P2PClient.
   @param signaling_channel Signaling channel used for exchange signaling messages.
   */
  P2PClient(P2PClientConfiguration& configuration,
            std::shared_ptr<P2PSignalingChannelInterface> signaling_channel);
  /*! Add an observer for peer client.
   @param observer Add this object to observer list.
          Do not delete this object until it is removed from observer list.
   */
  void AddObserver(P2PClientObserver& observer);
  /*! Remove an observer from peer client.
   @param observer Remove this object from observer list.
   */
  void RemoveObserver(P2PClientObserver& observer);
  /**
   @brief Connect to the signaling server.
   @param host The URL of signaling server to connect
   @param token A token used for connection and authentication
   @param on_success Sucess callback will be invoked with current user's ID if
          connect to server successfully.
   @param on_failure Failure callback will be invoked if one of these cases
                     happened:
                     1. P2PClient is connecting or connected to a server.
                     2. Invalid token.
   */
  void Connect(const std::string& host,
               const std::string& token,
               std::function<void(const std::string&)> on_success,
               std::function<void(std::unique_ptr<Exception>)> on_failure);
  /**
  @brief Disconnect from the signaling server.
         It will stop all active WebRTC sessions.
  @param on_success Sucess callback will be invoked if disconnect from server
                    successfully.
  @param on_failure Failure callback will be invoked if one of these cases
                    happened:
                    1. P2PClient hasn't connected to a signaling server.
   */
  void Disconnect(std::function<void()> on_success,
                  std::function<void(std::unique_ptr<Exception>)> on_failure);
  /**
  @brief Add a remote user to the allowed list to start a WebRTC session.
  @param target_id Remote user's ID.
   */
  void AddAllowedRemoteId(const std::string& target_id);
  /**
  @brief Remove a remote user from the allowed list to stop a WebRTC session.
  @param target_id Remote user's ID.
  @param on_success Success callback will be invoked if removing a remote user
  successfully.
  @param on_failure Failure callback will be invoked if one of the following
  cases happened.
      1. P2PClient is disconnected from the server.
      2. Target ID is null or target user is offline.
   */
  void RemoveAllowedRemoteId(const std::string& target_id,
                             std::function<void()> on_success,
                             std::function<void(std::unique_ptr<Exception>)> on_failure);
  /**
   @brief Stop a WebRTC session.
   @param target_id Remote user's ID.
   @param target_id Success callback will be invoked if send stop event
   successfully.
   @param on_failure Failure callback will be invoked if one of the following
   cases
   happened.
                  1. P2PClient is disconnected from the server.
                  2. Target ID is null or target user is offline.
                  3. There is no WebRTC session with target user.
   */
  void Stop(const std::string& target_id,
            std::function<void()> on_success,
            std::function<void(std::unique_ptr<Exception>)> on_failure);
  /**
   @brief Publish a stream to the remote client.
   @param stream The stream which will be published.
   @param target_id Target user's ID.
   @param on_success Success callback will be invoked it the stream is
   published.
   @param on_failure Failure callback will be invoked if one of these cases
   happened:
                    1. P2PClient is disconnected from server.
                    2. Target ID is null or user is offline.
                    3. Haven't connected to remote client.
   */
  void Publish(const std::string& target_id,
               std::shared_ptr<owt::base::LocalStream> stream,
               std::function<void(std::shared_ptr<P2PPublication>)> on_success,
               std::function<void(std::unique_ptr<Exception>)> on_failure);
  /**
   @brief Send a message to remote client
   @param target_id Remote user's ID.
   @param message The message to be sent.
   @param on_success Success callback will be invoked if send deny event
   successfully.
   @param on_failure Failure callback will be invoked if one of the following
   cases happened.
   1. P2PClient is disconnected from the server.
   2. Target ID is null or target user is offline.
   3. There is no WebRTC session with target user.
   */
  void Send(const std::string& target_id,
            const std::string& message,
            std::function<void()> on_success,
            std::function<void(std::unique_ptr<Exception>)> on_failure);
  /**
   @brief Get the connection statistowt with target client.
   @param target_id Remote user's ID.
   @param on_success Success callback will be invoked if get statistoms
   information successes.
   @param on_failure Failure callback will be invoked if one of the following
   cases happened.
   1. P2PClient is disconnected from the server.
   2. Target ID is invalid.
   3. There is no WebRTC session with target user.
   */
  void GetConnectionStats(
      const std::string& target_id,
      std::function<void(std::shared_ptr<owt::base::ConnectionStats>)> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);
  /** @cond */
  void SetLocalId(const std::string& local_id);
  /** @endcond */
 protected:
  // Implement P2PSignalingSenderInterface
  virtual void SendSignalingMessage(const std::string& message,
                                    const std::string& remote_id,
                                    std::function<void()> on_success,
                                    std::function<void(std::unique_ptr<Exception>)> on_failure);
  // Implement P2PSignalingChannelObserver
  virtual void OnMessage(const std::string& message, const std::string& sender);
  virtual void OnServerDisconnected();
  // Handle events from P2PPeerConnectionChannel
  // Triggered when the WebRTC session is started.
  virtual void OnStarted(const std::string& remote_id);
  // Triggered when the WebRTC session is ended.
  virtual void OnStopped(const std::string& remote_id);
  // Triggered when remote user denied the invitation.
  virtual void OnDenied(const std::string& remote_id);
  // Triggered when remote user send data via data channel.
  // Currently, data is string.
  virtual void OnData(const std::string& remote_id,
                      const std::string& message);
  // Triggered when a new stream is added.
  virtual void OnStreamAdded(
      std::shared_ptr<owt::base::RemoteStream> stream);
 private:
  void Unpublish(const std::string& target_id,
                 std::shared_ptr<LocalStream> stream,
                 std::function<void()> on_success,
                 std::function<void(std::unique_ptr<Exception>)> on_failure);
  std::shared_ptr<P2PPeerConnectionChannel> GetPeerConnectionChannel(
      const std::string& target_id);
  bool IsPeerConnectionChannelCreated(const std::string& target_id);
  owt::base::PeerConnectionChannelConfiguration GetPeerConnectionChannelConfiguration();
  // Queue for callbacks and events. Shared among P2PClient and all of it's
  // P2PPeerConnectionChannel.
  std::shared_ptr<rtc::TaskQueue> event_queue_;
  std::shared_ptr<P2PSignalingChannelInterface> signaling_channel_;
  std::unordered_map<std::string, std::shared_ptr<P2PPeerConnectionChannel>>
      pc_channels_;
  std::string local_id_;
  std::vector<std::reference_wrapper<P2PClientObserver>> observers_;
  P2PClientConfiguration configuration_;
  mutable std::mutex remote_ids_mutex_;
  std::vector<std::string> allowed_remote_ids_;
};
}
}
#endif  // OWT_P2P_P2PCLIENT_H_
