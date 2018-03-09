/*
 * Intel License
 */

#include <algorithm>
#include <future>

#include "webrtc/rtc_base/base64.h"
#include "webrtc/rtc_base/checks.h"
#include "webrtc/rtc_base/criticalsection.h"
#include "webrtc/rtc_base/logging.h"
#include "webrtc/rtc_base/task_queue.h"
#include "talk/ics/sdk/base/eventtrigger.h"
#include "talk/ics/sdk/base/stringutils.h"
#include "talk/ics/sdk/include/cpp/ics/base/stream.h"
#include "talk/ics/sdk/include/cpp/ics/p2p/p2pclient.h"
#include "talk/ics/sdk/p2p/p2ppeerconnectionchannel.h"
#include "talk/ics/sdk/p2p/p2ppeerconnectionchannelobservercppimpl.h"

namespace ics {
namespace p2p {

P2PClient::P2PClient(
    P2PClientConfiguration& configuration,
    std::shared_ptr<P2PSignalingChannelInterface> signaling_channel)
    : event_queue_(new rtc::TaskQueue("P2PClientEventQueue")),
      signaling_channel_(signaling_channel),
      configuration_(configuration) {
  RTC_CHECK(signaling_channel_);
  signaling_channel_->AddObserver(*this);
}

void P2PClient::Connect(
    const std::string& host,
    const std::string& token,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  RTC_CHECK(signaling_channel_);
  signaling_channel_->Connect(host, token, on_success, on_failure);
}

void P2PClient::Disconnect(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  RTC_CHECK(signaling_channel_);
  signaling_channel_->Disconnect(on_success, on_failure);
}

void P2PClient::AddAllowedRemoteId(const std::string& target_id) {
  if (std::find(allowed_remote_ids_.begin(), allowed_remote_ids_.end(), target_id) !=
      allowed_remote_ids_.end()) {
    LOG(LS_INFO) << "Adding duplicated remote id.";
    return;
  }

  const std::lock_guard<std::mutex> lock(remote_ids_mutex_);
  allowed_remote_ids_.push_back(target_id);
}

void P2PClient::RemoveAllowedRemoteId(const std::string& target_id,
                                      std::function<void()> on_success,
                                      std::function<void(std::unique_ptr<Exception>)> on_failure) {
  if (std::find(allowed_remote_ids_.begin(), allowed_remote_ids_.end(), target_id) ==
      allowed_remote_ids_.end()) {
    LOG(LS_INFO) << "Trying to delete non-existed remote id.";
    return;
  }

  Stop(target_id, on_success, on_failure);

  const std::lock_guard<std::mutex> lock(remote_ids_mutex_);
  allowed_remote_ids_.erase(
      std::remove(allowed_remote_ids_.begin(), allowed_remote_ids_.end(), target_id),
      allowed_remote_ids_.end());
}

void P2PClient::Send(
    const std::string& target_id,
    const std::string& message,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  auto pcc = GetPeerConnectionChannel(target_id);
  pcc->Send(message, on_success, on_failure);
}

void P2PClient::Publish(
    const std::string& target_id,
    std::shared_ptr<LocalStream> stream,
    std::function<void(std::shared_ptr<P2PPublication>)> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  // First check whether target_id is in the allowed_remote_ids_ list
  if (std::find(allowed_remote_ids_.begin(), allowed_remote_ids_.end(), target_id) ==
      allowed_remote_ids_.end()) {
    LOG(LS_WARNING) << "Publishing cannot be done since the remote user is not allowed.";
    return;
  }

  // Second invite the remote user to the chat
  auto pcc = GetPeerConnectionChannel(target_id);
  pcc->Invite(nullptr, nullptr);

  // Then do the real publication
  std::weak_ptr<P2PClient> weak_this = shared_from_this();
  pcc->Publish(stream, [on_success, weak_this, target_id, stream] () {
    auto that = weak_this.lock();
    if (!that)
      return;

    std::shared_ptr<P2PPublication> publication(new P2PPublication(that, target_id, stream));
    on_success(publication);
  }, on_failure);
}

void P2PClient::Unpublish(
    const std::string& target_id,
    std::shared_ptr<LocalStream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  auto pcc = GetPeerConnectionChannel(target_id);
  pcc->Unpublish(stream, on_success, on_failure);
}

void P2PClient::Stop(
    const std::string& target_id,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  auto pcc = GetPeerConnectionChannel(target_id);
  pcc->Stop(on_success, on_failure);
}

void P2PClient::GetConnectionStats(
    const std::string& target_id,
    std::function<void(std::shared_ptr<ics::base::ConnectionStats>)>
        on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  auto pcc = GetPeerConnectionChannel(target_id);
  pcc->GetConnectionStats(on_success, on_failure);
}

void P2PClient::OnMessage(const std::string& message,
                           const std::string& sender) {
  auto pcc = GetPeerConnectionChannel(sender);
  pcc->OnIncomingSignalingMessage(message);
}

void P2PClient::OnDisconnected() {
  // TODO: Implement disconnected handler.
  LOG(LS_INFO) << "Disconnected from signaling server.";
}

void P2PClient::SendSignalingMessage(const std::string& message,
                                      const std::string& remote_id,
                                      std::function<void()> on_success,
                                      std::function<void(int)> on_failure) {
  signaling_channel_->SendMessage(message, remote_id, on_success,
                                  nullptr);  // TODO:fix on_failure.
}

void P2PClient::AddObserver(P2PClientObserver& observer) {
  observers_.push_back(observer);
}

void P2PClient::RemoveObserver(P2PClientObserver& observer) {
  observers_.erase(std::find_if(
      observers_.begin(), observers_.end(),
      [&](std::reference_wrapper<P2PClientObserver> o) -> bool {
        return &observer == &(o.get());
      }));
}

std::shared_ptr<P2PPeerConnectionChannel> P2PClient::GetPeerConnectionChannel(
    const std::string& target_id) {
  auto pcc_it = pc_channels_.find(target_id);
  if (pcc_it == pc_channels_.end()) {  // Create new channel if it doesn't exist
    PeerConnectionChannelConfiguration config =
        GetPeerConnectionChannelConfiguration();
    std::shared_ptr<P2PPeerConnectionChannel> pcc =
        std::shared_ptr<P2PPeerConnectionChannel>(new P2PPeerConnectionChannel(
            config, local_id_, target_id, this, event_queue_));
    P2PPeerConnectionChannelObserverCppImpl* pcc_observer =
        new P2PPeerConnectionChannelObserverCppImpl(*this);
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

PeerConnectionChannelConfiguration P2PClient::GetPeerConnectionChannelConfiguration() {
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
  // TODO: configuration structure changed.
  // config.media_codec = configuration_.media_codec;
  config.candidate_network_policy =
      (configuration_.candidate_network_policy ==
       ClientConfiguration::CandidateNetworkPolicy::kLowCost)
          ? webrtc::PeerConnectionInterface::CandidateNetworkPolicy::
                kCandidateNetworkPolicyLowCost
          : webrtc::PeerConnectionInterface::CandidateNetworkPolicy::
                kCandidateNetworkPolicyAll;
  return config;
}

void P2PClient::OnInvited(const std::string& remote_id) {
  EventTrigger::OnEvent1(observers_, event_queue_,
                         &P2PClientObserver::OnInvited, remote_id);

  // First check whether remote_id is in the allowed_remote_ids_ list
  if (std::find(allowed_remote_ids_.begin(), allowed_remote_ids_.end(), remote_id) ==
        allowed_remote_ids_.end()) {
      LOG(LS_WARNING) << "Chat cannot be setup since the remote user is not allowed.";
  } else {
    // Then accept the chat if allowed
    auto pcc = GetPeerConnectionChannel(remote_id);
    pcc->Accept(nullptr, nullptr);
  }
}

void P2PClient::OnAccepted(const std::string& remote_id) {
  EventTrigger::OnEvent1(observers_, event_queue_,
                         &P2PClientObserver::OnAccepted, remote_id);
}

void P2PClient::OnStarted(const std::string& remote_id) {
  EventTrigger::OnEvent1(observers_, event_queue_,
                         &P2PClientObserver::OnChatStarted, remote_id);
}

void P2PClient::OnStopped(const std::string& remote_id) {
  pc_channels_.erase(remote_id);
  EventTrigger::OnEvent1(observers_, event_queue_,
                         &P2PClientObserver::OnChatStopped, remote_id);
}

void P2PClient::OnDenied(const std::string& remote_id) {
  pc_channels_.erase(remote_id);
  EventTrigger::OnEvent1(observers_, event_queue_,
                         &P2PClientObserver::OnDenied, remote_id);
}

void P2PClient::OnData(const std::string& remote_id,
                        const std::string& message) {
  EventTrigger::OnEvent2(observers_, event_queue_,
                         &P2PClientObserver::OnDataReceived, remote_id,
                         message);
}

void P2PClient::OnStreamAdded(std::shared_ptr<RemoteStream> stream) {
  EventTrigger::OnEvent1(
      observers_, event_queue_,
      (void (P2PClientObserver::*)(std::shared_ptr<RemoteStream>))(
          &P2PClientObserver::OnStreamAdded),
      stream);
}

void P2PClient::OnStreamRemoved(std::shared_ptr<RemoteStream> stream) {
  EventTrigger::OnEvent1(
      observers_, event_queue_,
      (void (P2PClientObserver::*)(std::shared_ptr<RemoteStream>))(
          &P2PClientObserver::OnStreamRemoved),
      stream);
}
}
}
