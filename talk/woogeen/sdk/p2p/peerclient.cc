/*
 * Intel License
 */

#include <future>
#include "talk/woogeen/sdk/p2p/p2ppeerconnectionchannel.h"
#include "talk/woogeen/sdk/p2p/p2ppeerconnectionchannelobservercppimpl.h"
#include "talk/woogeen/sdk/include/cpp/woogeen/base/stream.h"
#include "talk/woogeen/sdk/include/cpp/woogeen/p2p/peerclient.h"
#include "webrtc/base/checks.h"

namespace woogeen {
namespace p2p {

PeerClient::PeerClient(
    PeerClientConfiguration& configuration,
    std::shared_ptr<P2PSignalingChannelInterface> signaling_channel)
    : signaling_channel_(signaling_channel), configuration_(configuration) {
  RTC_CHECK(signaling_channel_);
  signaling_channel_->AddObserver(*this);
}

// TODO(jianjunz): Remove them to utility class
template <typename T1, typename T2>
void PeerClient::OnEvent1(T1 func, T2 arg1) {
  for (auto it = observers_.begin(); it != observers_.end(); ++it) {
    auto f = std::bind(func, *it, arg1);
    std::async(std::launch::async, f);
  }
}

template <typename T1, typename T2, typename T3>
void PeerClient::OnEvent2(T1 func, T2 arg1, T3 arg2) {
  for (auto it = observers_.begin(); it != observers_.end(); ++it) {
    auto f = std::bind(func, *it, arg1, arg2);
    std::async(std::launch::async, f);
  }
}

void PeerClient::Connect(
    const std::string& token,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<P2PException>)> on_failure) {
  RTC_CHECK(signaling_channel_);
  signaling_channel_->Connect(token, on_success, on_failure);
}

void PeerClient::Disconnect(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<P2PException>)> on_failure) {
  RTC_CHECK(signaling_channel_);
  signaling_channel_->Disconnect(on_success, on_failure);
}

void PeerClient::Invite(
    const std::string& target_id,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<P2PException>)> on_failure) {
  auto pcc = GetPeerConnectionChannel(target_id);
  pcc->Invite(on_success, on_failure);
}

void PeerClient::Send(
    const std::string& target_id,
    const std::string& message,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<P2PException>)> on_failure) {
  auto pcc = GetPeerConnectionChannel(target_id);
  pcc->Send(message, on_success, on_failure);
}

void PeerClient::Publish(
    const std::string& target_id,
    std::shared_ptr<LocalStream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<P2PException>)> on_failure) {
  auto pcc = GetPeerConnectionChannel(target_id);
  pcc->Publish(stream, on_success, on_failure);
}

void PeerClient::Unpublish(
    const std::string& target_id,
    std::shared_ptr<LocalStream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<P2PException>)> on_failure) {
  auto pcc = GetPeerConnectionChannel(target_id);
  pcc->Unpublish(stream, on_success, on_failure);
}

void PeerClient::Stop(
    const std::string& target_id,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<P2PException>)> on_failure) {
  auto pcc = GetPeerConnectionChannel(target_id);
  pcc->Stop(on_success, on_failure);
}

void PeerClient::OnMessage(const std::string& message,
                           const std::string& sender) {
  auto pcc = GetPeerConnectionChannel(sender);
  pcc->OnIncomingSignalingMessage(message);
}

void PeerClient::OnDisconnected() {
  // TODO: Implement disconnected handler.
  LOG(LS_INFO) << "Disconnected from signaling server.";
}

void PeerClient::SendSignalingMessage(const std::string& message,
                                      const std::string& remote_id,
                                      std::function<void()> on_success,
                                      std::function<void(int)> on_failure) {
  signaling_channel_->SendMessage(message, remote_id, on_success,
                                  nullptr);  // TODO:fix on_failure.
}

void PeerClient::AddObserver(PeerClientObserver& observer) {
  observers_.push_back(observer);
}

void PeerClient::RemoveObserver(PeerClientObserver& observer) {
  observers_.erase(std::find_if(
      observers_.begin(), observers_.end(),
      [&](std::reference_wrapper<PeerClientObserver> o) -> bool {
        return &observer == &(o.get());
      }));
}

std::shared_ptr<P2PPeerConnectionChannel> PeerClient::GetPeerConnectionChannel(
    const std::string& target_id) {
  auto pcc_it = pc_channels_.find(target_id);
  if (pcc_it == pc_channels_.end()) {  // Create new channel if it doesn't exist
    PeerConnectionChannelConfiguration config =
        GetPeerConnectionChannelConfiguration();
    std::shared_ptr<P2PPeerConnectionChannel> pcc =
        std::shared_ptr<P2PPeerConnectionChannel>(
            new P2PPeerConnectionChannel(config, local_id_, target_id, this));
    P2PPeerConnectionChannelObserverCppImpl *pcc_observer=new P2PPeerConnectionChannelObserverCppImpl(*this);
    pcc->AddObserver(pcc_observer);
    auto pcc_pair =
        std::pair<std::string, std::shared_ptr<P2PPeerConnectionChannel>>(
            target_id, pcc);
    pc_channels_.insert(pcc_pair);
    auto pcc_observer_pair =
        std::pair<std::string, P2PPeerConnectionChannelObserverCppImpl*>(
            target_id, pcc_observer);
    pcc_observers_.insert(pcc_observer_pair);
    return pcc;
  } else {
    return pcc_it->second;
  }
}

PeerConnectionChannelConfiguration PeerClient::GetPeerConnectionChannelConfiguration() {
  PeerConnectionChannelConfiguration config;
  std::vector<webrtc::PeerConnectionInterface::IceServer> ice_servers;
  for(auto it = configuration_.ice_servers.begin(); it!=configuration_.ice_servers.end();++it){
    webrtc::PeerConnectionInterface::IceServer ice_server;
    ice_server.urls=(*it).urls;
    ice_server.username=(*it).username;
    ice_server.password=(*it).password;
    ice_servers.push_back(ice_server);
  }
  config.servers = ice_servers;
  config.media_codec = configuration_.media_codec;
  config.encoded_video_frame_ = configuration_.encoded_video_frame_;
  return config;
}

void PeerClient::OnInvited(const std::string& remote_id) {
  OnEvent1(&PeerClientObserver::OnInvited, remote_id);
}

void PeerClient::OnAccepted(const std::string& remote_id) {
  OnEvent1(&PeerClientObserver::OnAccepted, remote_id);
}

void PeerClient::OnStarted(const std::string& remote_id) {
  OnEvent1(&PeerClientObserver::OnChatStarted, remote_id);
}

void PeerClient::OnStopped(const std::string& remote_id) {
  OnEvent1(&PeerClientObserver::OnChatStopped, remote_id);
}

void PeerClient::OnDenied(const std::string& remote_id) {
  OnEvent1(&PeerClientObserver::OnDenied, remote_id);
}

void PeerClient::OnData(const std::string& remote_id,
                        const std::string& message) {
  OnEvent2(&PeerClientObserver::OnDataReceived, remote_id, message);
}

void PeerClient::OnStreamAdded(
    std::shared_ptr<RemoteCameraStream> stream) {
  OnEvent1((void(PeerClientObserver::*)(
               std::shared_ptr<RemoteCameraStream>))(
               &PeerClientObserver::OnStreamAdded),
           stream);
}
void PeerClient::OnStreamAdded(
    std::shared_ptr<RemoteScreenStream> stream) {
  OnEvent1((void(PeerClientObserver::*)(
               std::shared_ptr<RemoteScreenStream>))(
               &PeerClientObserver::OnStreamAdded),
           stream);
}

void PeerClient::OnStreamRemoved(
    std::shared_ptr<RemoteCameraStream> stream) {
  OnEvent1(
      (void (
          PeerClientObserver::*)(std::shared_ptr<RemoteCameraStream>))(
          &PeerClientObserver::OnStreamRemoved),
      stream);
}

void PeerClient::OnStreamRemoved(
    std::shared_ptr<RemoteScreenStream> stream) {
  OnEvent1(
      (void (
          PeerClientObserver::*)(std::shared_ptr<RemoteScreenStream>))(
          &PeerClientObserver::OnStreamRemoved),
      stream);
}
}
}
