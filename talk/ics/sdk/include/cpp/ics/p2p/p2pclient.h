/*
 * Copyright © 2018 Intel Corporation. All Rights Reserved.
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

#ifndef ICS_P2P_P2PCLIENT_H_
#define ICS_P2P_P2PCLIENT_H_

#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <set>

#include "ics/base/commontypes.h"
#include "ics/base/connectionstats.h"
#include "ics/base/stream.h"
#include "ics/p2p/p2ppublication.h"
#include "ics/p2p/p2psignalingchannelinterface.h"
#include "ics/p2p/p2psignalingsenderinterface.h"
#include "ics/base/clientconfiguration.h"
#include "ics/base/globalconfiguration.h"

namespace rtc {
  class TaskQueue;
}

namespace ics {
namespace base {
  struct PeerConnectionChannelConfiguration;
}

namespace p2p{

/**
 @brief Configuration for P2PClient

 This configuration is used while creating P2PClient. Changing this
 configuration does NOT impact P2PClient already created.
*/
struct P2PClientConfiguration : ics::base::ClientConfiguration {
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
      std::shared_ptr<ics::base::RemoteStream> stream){};
  /**
   @brief This function will be invoked when a remote stream is removed.
   @param stream The remote stream removed.
   */
  virtual void OnStreamRemoved(
      std::shared_ptr<ics::base::RemoteStream> stream){};
};

/// An async client for P2P WebRTC sessions
class P2PClient final
    : protected P2PSignalingSenderInterface,
      protected P2PSignalingChannelInterfaceObserver,
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
               std::function<void()> on_success,
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
               std::shared_ptr<ics::base::LocalStream> stream,
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
   @brief Get the connection statistics with target client.
   @param target_id Remote user's ID.
   @param on_success Success callback will be invoked if get statistics
   information successes.
   @param on_failure Failure callback will be invoked if one of the following
   cases happened.
   1. P2PClient is disconnected from the server.
   2. Target ID is invalid.
   3. There is no WebRTC session with target user.
   */
  void GetConnectionStats(
      const std::string& target_id,
      std::function<void(std::shared_ptr<ics::base::ConnectionStats>)> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure);

 protected:
  // Implement P2PSignalingSenderInterface
  virtual void SendSignalingMessage(const std::string& message,
                                    const std::string& remote_id,
                                    std::function<void()> on_success,
                                    std::function<void(int)> on_failure);
  // Implement P2PSignalingChannelInterfaceObserver
  virtual void OnMessage(const std::string& message, const std::string& sender);
  virtual void OnDisconnected();

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
      std::shared_ptr<ics::base::RemoteStream> stream);
  // Triggered when a remote stream is removed.
  virtual void OnStreamRemoved(
      std::shared_ptr<ics::base::RemoteStream> stream);

 private:
  void Unpublish(const std::string& target_id,
                 std::shared_ptr<LocalStream> stream,
                 std::function<void()> on_success,
                 std::function<void(std::unique_ptr<Exception>)> on_failure);
  std::shared_ptr<P2PPeerConnectionChannel> GetPeerConnectionChannel(
      const std::string& target_id);
  ics::base::PeerConnectionChannelConfiguration GetPeerConnectionChannelConfiguration();

  // Queue for callbacks and events. Shared among P2PClient and all of it's
  // P2PPeerConnectionChannel.
  std::shared_ptr<rtc::TaskQueue> event_queue_;
  std::shared_ptr<P2PSignalingChannelInterface> signaling_channel_;
  std::unordered_map<std::string, std::shared_ptr<P2PPeerConnectionChannel>>
      pc_channels_;
  // P2PPeerConnectionChannelObserver adapter for each PeerConnectionChannel
  std::unordered_map<std::string, P2PPeerConnectionChannelObserverCppImpl*>
      pcc_observers_;
  std::string local_id_;
  std::vector<std::reference_wrapper<P2PClientObserver>> observers_;
  // It receives events from P2PPeerConnectionChannel and notify P2PClient.
  P2PPeerConnectionChannelObserverCppImpl* pcc_observer_impl_;
  P2PClientConfiguration configuration_;

  mutable std::mutex remote_ids_mutex_;
  std::vector<std::string> allowed_remote_ids_;
};

}
}

#endif  // ICS_P2P_P2PCLIENT_H_
