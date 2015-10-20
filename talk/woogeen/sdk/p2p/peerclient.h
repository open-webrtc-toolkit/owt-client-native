/*
 * Intel License
 */

#ifndef WOOGEEN_P2P_P2PPEERCLIENT_H_
#define WOOGEEN_P2P_P2PPEERCLIENT_H_

#include <memory>
#include <unordered_map>
#include "talk/woogeen/sdk/p2p/p2ppeerconnectionchannel.h"
#include "talk/woogeen/sdk/p2p/p2psignalingchannelinterface.h"
#include "talk/woogeen/sdk/p2p/p2psignalingsenderinterface.h"
#include "talk/woogeen/sdk/base/stream.h"

namespace woogeen {

/// Observer for PeerClient
class PeerClientObserver {
 public:
  /**
   @brief This function will be invoked when client is disconnected from
   signaling
   server.
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
      std::shared_ptr<woogeen::RemoteCameraStream> stream){};
  /**
   @brief This function will be invoked when a remote stream is available.
   @param stream The remote stream added.
   */
  virtual void OnStreamAdded(
      std::shared_ptr<woogeen::RemoteScreenStream> stream){};
  /**
   @brief This function will be invoked when a remote stream is removed.
   @param stream The remote stream removed.
   */
  virtual void OnStreamRemoved(
      std::shared_ptr<woogeen::RemoteCameraStream> stream){};
  /**
   @brief This function will be invoked when a remote stream is removed.
   @param stream The remote stream removed.
   */
  virtual void OnStreamRemoved(
      std::shared_ptr<woogeen::RemoteScreenStream> stream){};
};

/// An async client for P2P WebRTC sessions
class PeerClient : protected P2PSignalingSenderInterface,
                   protected P2PSignalingChannelInterfaceObserver {
 public:
  PeerClient(std::shared_ptr<P2PSignalingChannelInterface> signaling_channel);
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
   @param targetId Remote user's ID.
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
               std::shared_ptr<LocalStream> stream,
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
                 std::shared_ptr<LocalStream> stream,
                 std::function<void()> on_success,
                 std::function<void(std::unique_ptr<P2PException>)> on_failure);

  /**
   @brief Send a message to remote client
   @param targetId Remote user's ID.
   @param message The message to be sent.
   @param on_success Success callback will be invoked if send deny event
   successfully.
   @param on_failure Failure callback will be invoked if one of the following
   cases
   happened.
   1. PeerClient is disconnected from the server.
   2. Target ID is null or target user is offline.
   3. There is no WebRTC session with target user.
   */
  void Send(const std::string& target_id,
            const std::string& message,
            std::function<void()> on_success,
            std::function<void(std::unique_ptr<P2PException>)> on_failure);

 protected:
  // Implement
  virtual void SendSignalingMessage(const std::string& message,
                                    const std::string& remote_id,
                                    std::function<void()> on_success,
                                    std::function<void(int)> on_failure);
  // Implement P2PSignalingChannelInterfaceObserver
  virtual void OnMessage(const std::string& message, const std::string& sender);
  virtual void OnDisconnected();

 private:
  std::shared_ptr<P2PPeerConnectionChannel> GetPeerConnectionChannel(
      const std::string& target_id);

  std::shared_ptr<P2PSignalingChannelInterface> signaling_channel_;
  std::unordered_map<std::string, std::shared_ptr<P2PPeerConnectionChannel>>
      pc_channels_;
  std::string local_id_;
};
}

#endif  // WOOGEEN_P2P_P2PPEERCLIENT_H_