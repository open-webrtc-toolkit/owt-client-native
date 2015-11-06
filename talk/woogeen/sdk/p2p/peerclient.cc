/*
 * Intel License
 */

#include "talk/woogeen/sdk/base/stream.h"
#include "talk/woogeen/sdk/p2p/peerclient.h"
#include "talk/woogeen/sdk/p2p/p2ppeerconnectionchannel.h"
#include "webrtc/base/checks.h"

namespace woogeen {

PeerClient::PeerClient(
    std::shared_ptr<P2PSignalingChannelInterface> signaling_channel)
    : signaling_channel_(signaling_channel) {
  RTC_CHECK(signaling_channel_);
  signaling_channel_->AddObserver(this);
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

void PeerClient::OnMessage(const std::string& message,
                           const std::string& sender) {
  auto pcc = GetPeerConnectionChannel(sender);
  pcc->OnIncomingSignalingMessage(message);
}

void PeerClient::OnDisconnected() {
  // TODO: Implement disconnected handler.
  LOG(LS_INFO) << "Disconnected from signaling server.";
}

// Windows defines SendMessage as SendMessageA or SendMessageW. Undefine it
// here and redefined it later.
#if defined(UNICODE) && defined(WEBRTC_WIN) && defined(SendMessage)
#undef SendMessage
#endif

void PeerClient::SendSignalingMessage(const std::string& message,
                                      const std::string& remote_id,
                                      std::function<void()> on_success,
                                      std::function<void(int)> on_failure) {
  signaling_channel_->SendMessage(message, remote_id, on_success,
                                  nullptr);  // TODO:fix on_failure.
}

#ifdef WEBRTC_WIN
#ifdef UNICODE
#define SendMessage SendMessageW
#else
#define SendMessage SendMessageA
#endif
#endif

std::shared_ptr<P2PPeerConnectionChannel> PeerClient::GetPeerConnectionChannel(
    const std::string& target_id) {
  auto pcc_it = pc_channels_.find(target_id);
  if (pcc_it == pc_channels_.end()) {  // Create new channel if it doesn't exist
    PeerConnectionChannelConfiguration config;
    std::shared_ptr<P2PPeerConnectionChannel> pcc =
        std::shared_ptr<P2PPeerConnectionChannel>(
            new P2PPeerConnectionChannel(config, local_id_, target_id, this));
    auto pcc_pair =
        std::pair<std::string, std::shared_ptr<P2PPeerConnectionChannel>>(
            target_id, pcc);
    pc_channels_.insert(pcc_pair);
    return pcc;
  } else {
    return pcc_it->second;
  }
}
}
