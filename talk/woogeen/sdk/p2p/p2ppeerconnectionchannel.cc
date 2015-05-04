/*
 * Intel License
 */

#include "p2ppeerconnectionchannel.h"

namespace woogeen {

using std::string;

enum P2PPeerConnectionChannel::State : int {
  kReady = 1,  // Indicate the channel is ready. This is the initial state.
  kOffered,  // Indicates local client has sent an invitation and waiting for an acceptance.
  kPending,  // Indicates local client received an invitation and waiting for user's response.
  kMatched,  // Indicates both sides agreed to start a WebRTC session. One of them will send an offer soon.
  kConnecting,  // Indicates both sides are trying to connect to the other side.
  kConnected,  // Indicates PeerConnection has been established.
};

static const string kMessageTypeKey = "type";
static const string kChatInvitation = "chat-invitation";
static const string kChatDeny = "chat-denied";
static const string kChatStop = "chat-closed";
static const string kChatSignal = "chat-signal";
static const string kStreamType = "stream-type";

P2PPeerConnectionChannel::P2PPeerConnectionChannel(const std::string& remote_id, SignalingSenderInterface* sender)
    :signaling_sender_(sender),
     remote_id_(remote_id),
     state_(kReady) {
  CHECK(signaling_sender_);
}

void P2PPeerConnectionChannel::Invite(std::function<void()> success, std::function<void(int)> failure) {
  Json::Value json;
  json[kMessageTypeKey]=kChatInvitation;
  SendSignalingMessage(json, success, failure);
}

void P2PPeerConnectionChannel::OnIncomingMessage(const std::string& message) {
}

void P2PPeerConnectionChannel::ChangeState(State state) {
  state_=state;
}

void P2PPeerConnectionChannel::SendSignalingMessage(const Json::Value& data, std::function<void()> success, std::function<void(int)> failure) {
  CHECK(signaling_sender_);
  std::string jsonString=JsonValueToString(data);
  signaling_sender_->Send(jsonString, remote_id_, success, failure);
}
}
