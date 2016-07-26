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

#ifndef WOOGEEN_P2P_P2PPEERCLIENT_H_
#define WOOGEEN_P2P_P2PPEERCLIENT_H_

#include <memory>
#include <unordered_map>
#include <iostream>
#include <vector>
#include "woogeen/base/connectionstats.h"
#include "woogeen/base/stream.h"
#include "woogeen/p2p/p2psignalingchannelinterface.h"
#include "woogeen/p2p/p2psignalingsenderinterface.h"
#include "woogeen/base/clientconfiguration.h"
#include "woogeen/base/globalconfiguration.h"

namespace woogeen {
namespace base {
  struct PeerConnectionChannelConfiguration;
}

namespace p2p{

/**
 @brief Configuration for PeerClient

 This configuration is used while creating PeerClient. Changing this
 configuration does NOT impact PeerClient already created.
*/
struct PeerClientConfiguration : woogeen::base::ClientConfiguration {};

class P2PPeerConnectionChannelObserverCppImpl;
class P2PPeerConnectionChannel;

/// Observer for PeerClient
class PeerClientObserver {
 public:
  /**
   @brief This function will be invoked when client is disconnected from
   signaling server.
   */
  virtual void OnServerDisconnected(){};
  /**
   @brief This function will be invoked when received a invitation.
   @param remote_user_id Remote user’s ID
   */
  virtual void OnInvited(const std::string& remote_user_id){};
  /**
   @brief This function will be invoked when a remote user denied current user's
   invitation.
   @param remote_user_id Remote user’s ID
   */
  virtual void OnDenied(const std::string& remote_user_id){};
  /**
   @brief This function will be invoked when a remote user accepted current
   user's
   invitation.
   @param remote_user_id Remote user’s ID
   */
  virtual void OnAccepted(const std::string& remote_user_id){};
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
      std::shared_ptr<woogeen::base::RemoteCameraStream> stream){};
  /**
   @brief This function will be invoked when a remote stream is available.
   @param stream The remote stream added.
   */
  virtual void OnStreamAdded(
      std::shared_ptr<woogeen::base::RemoteScreenStream> stream){};
  /**
   @brief This function will be invoked when a remote stream is removed.
   @param stream The remote stream removed.
   */
  virtual void OnStreamRemoved(
      std::shared_ptr<woogeen::base::RemoteCameraStream> stream){};
  /**
   @brief This function will be invoked when a remote stream is removed.
   @param stream The remote stream removed.
   */
  virtual void OnStreamRemoved(
      std::shared_ptr<woogeen::base::RemoteScreenStream> stream){};
};

/// An async client for P2P WebRTC sessions
class PeerClient : protected P2PSignalingSenderInterface,
                   protected P2PSignalingChannelInterfaceObserver {

 public:
  /**
   @brief Init an PeerClient instance with speficied signaling channel.
   @param configuration Configuration for creating the PTCPeerClient.
   @param signalingChannel Signaling channel used for exchange signaling messages.
   */
  PeerClient(PeerClientConfiguration& configuration,
             std::shared_ptr<P2PSignalingChannelInterface> signaling_channel);
  /**
  @brief Connect to the signaling server.
  @param token A token used for connection and authentication
  @param on_success Sucess callback will be invoked with current user's ID if
         connect to server successfully.
  @param on_failure Failure callback will be invoked if one of these cases
                    happened:
                    1. PeerClient is connecting or connected to a server.
                    2. Invalid token.
  */
  void Connect(const std::string& token,
               std::function<void()> on_success,
               std::function<void(std::unique_ptr<P2PException>)> on_failure);
  /**
  @brief Disconnect from the signaling server.

         It will stop all active WebRTC sessions.
  @param on_success Sucess callback will be invoked if disconnect from server
                   successfully.
  @param on_failure Failure callback will be invoked if one of these cases
                   happened:
                   1. PeerClient haven't connected to a signaling server.
  */
  void Disconnect(
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<P2PException>)> on_failure);

  /**
  @brief Invite a remote user to start a WebRTC session.
  @param target_id Remote user's ID.
  @param on_success Success callback will be invoked if send a invitation
  successfully.
  @param on_failure Failure callback will be invoked if one of the following
  cases
  happened.
                1. PeerClient is disconnected from the server.
                2. Target ID is null or target user is offline.
  */
  void Invite(const std::string& target_id,
              std::function<void()> on_success,
              std::function<void(std::unique_ptr<P2PException>)> on_failure);

  /**
   @brief Accept a remote user's request to start a WebRTC session.
   @param target_id Remote user's ID.
   @param on_success Success callback will be invoked if send an acceptance
   successfully.
   @param on_failure Failure callback will be invoked if one of the following
   cases
   happened.
                  1. PeerClient is disconnected from the server.
                  2. Target ID is null or target user is offline.
                  3. Haven't received an invitation from target user.
   */
  void Accept(const std::string& target_id,
              std::function<void()> on_success,
              std::function<void(std::unique_ptr<P2PException>)> on_failure);

  /**
   @brief Deny a remote user's request to start a WebRTC session.
   @param target_id Remote user's ID.
   @param on_success Success callback will be invoked if send deny event
   successfully.
   @param on_failure Failure callback will be invoked if one of the following
   cases
   happened.
                  1. PeerClient is disconnected from the server.
                  2. Target ID is null or target user is offline.
                  3. Haven't received an invitation from target user.
   */
  void Deny(const std::string& target_id,
            std::function<void()> on_success,
            std::function<void(std::unique_ptr<P2PException>)> on_failure);

  /**
   @brief Stop a WebRTC session.
   @param target_id Remote user's ID.
   @param target_id Success callback will be invoked if send stop event
   successfully.
   @param on_failure Failure callback will be invoked if one of the following
   cases
   happened.
                  1. PeerClient is disconnected from the server.
                  2. Target ID is null or target user is offline.
                  3. There is no WebRTC session with target user.
   */
  void Stop(const std::string& target_id,
            std::function<void()> on_success,
            std::function<void(std::unique_ptr<P2PException>)> on_failure);

  /**
   @brief Publish a stream to the remote client.
   @param stream The stream which will be published.
   @param target_id Target user's ID.
   @param on_success Success callback will be invoked it the stream is
   published.
   @param on_failure Failure callback will be invoked if one of these cases
   happened:
                    1. PeerClient is disconnected from server.
                    2. Target ID is null or user is offline.
                    3. Haven't connected to remote client.
   */
  void Publish(const std::string& target_id,
               std::shared_ptr<woogeen::base::LocalStream> stream,
               std::function<void()> on_success,
               std::function<void(std::unique_ptr<P2PException>)> on_failure);

  /**
   @brief Unpublish the stream to the remote client.
   @param stream The stream which will be removed.
   @param target_id Target user's ID.
   @param on_success Success callback will be invoked it the stream is
   unpublished.
   @param on_failure Failure callback will be invoked if one of these cases
   happened:
                   1. PeerClient is disconnected from server.
                   2. Target ID is null or user is offline.
                   3. Haven't connected to remote client.
                   4. The stream haven't been published.
   */
  void Unpublish(const std::string& target_id,
                 std::shared_ptr<woogeen::base::LocalStream> stream,
                 std::function<void()> on_success,
                 std::function<void(std::unique_ptr<P2PException>)> on_failure);

  /**
   @brief Send a message to remote client
   @param target_id Remote user's ID.
   @param message The message to be sent.
   @param on_success Success callback will be invoked if send deny event
   successfully.
   @param on_failure Failure callback will be invoked if one of the following
   cases happened.
   1. PeerClient is disconnected from the server.
   2. Target ID is null or target user is offline.
   3. There is no WebRTC session with target user.
   */
  void Send(const std::string& target_id,
            const std::string& message,
            std::function<void()> on_success,
            std::function<void(std::unique_ptr<P2PException>)> on_failure);

  /**
   @brief Get the connection statistics with target client.
   @param target_id Remote user's ID.
   @param on_success Success callback will be invoked if get statistics
   information successes.
   @param on_failure Failure callback will be invoked if one of the following
   cases happened.
   1. PeerClient is disconnected from the server.
   2. Target ID is invalid.
   3. There is no WebRTC session with target user.
  */
  void GetConnectionStats(
      const std::string& target_id,
      std::function<void(std::shared_ptr<woogeen::base::ConnectionStats>)> on_success,
      std::function<void(std::unique_ptr<P2PException>)> on_failure);

  /*! Add an observer for peer client.
    @param observer Add this object to observer list.
                    Do not delete this object until it is removed from observer
                    list.
  */
  void AddObserver(PeerClientObserver& observer);

  /*! Remove an observer from peer client.
    @param observer Remove this object from observer list.
  */
  void RemoveObserver(PeerClientObserver& observer);

 protected:
  // Implement
  virtual void SendSignalingMessage(const std::string& message,
                                    const std::string& remote_id,
                                    std::function<void()> on_success,
                                    std::function<void(int)> on_failure);
  // Implement P2PSignalingChannelInterfaceObserver
  virtual void OnMessage(const std::string& message, const std::string& sender);
  virtual void OnDisconnected();

  // Handle events from P2PPeerConnectionChannel
  // Triggered when received an invitation.
  virtual void OnInvited(const std::string& remote_id);
  // Triggered when remote user accepted the invitation.
  virtual void OnAccepted(const std::string& remote_id);
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
      std::shared_ptr<woogeen::base::RemoteCameraStream> stream);
  virtual void OnStreamAdded(
      std::shared_ptr<woogeen::base::RemoteScreenStream> stream);
  // Triggered when a remote stream is removed.
  virtual void OnStreamRemoved(
      std::shared_ptr<woogeen::base::RemoteCameraStream> stream);
  virtual void OnStreamRemoved(
      std::shared_ptr<woogeen::base::RemoteScreenStream> stream);

 private:
  std::shared_ptr<P2PPeerConnectionChannel> GetPeerConnectionChannel(
      const std::string& target_id);
  woogeen::base::PeerConnectionChannelConfiguration
  GetPeerConnectionChannelConfiguration();

  std::shared_ptr<P2PSignalingChannelInterface> signaling_channel_;
  std::unordered_map<std::string, std::shared_ptr<P2PPeerConnectionChannel>>
      pc_channels_;
  // P2PPeerConnectionChannelObserver adapter for each PeerConnectionChannel
  std::unordered_map<std::string, P2PPeerConnectionChannelObserverCppImpl*>
      pcc_observers_;
  std::string local_id_;
  std::vector<std::reference_wrapper<PeerClientObserver>> observers_;
  // It receives events from P2PPeerConnectionChannel and notify PeerClient.
  P2PPeerConnectionChannelObserverCppImpl* pcc_observer_impl_;
  PeerClientConfiguration configuration_;

  friend class P2PPeerConnectionChannelObserverCppImpl;
};
}
}

#endif  // WOOGEEN_P2P_P2PPEERCLIENT_H_
