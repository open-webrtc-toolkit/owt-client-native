/*
 * Intel License
 */

#include <vector>
#include "webrtc/base/logging.h"
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

const string kMessageTypeKey = "type";
const string kChatInvitation = "chat-invitation";
const string kChatAccept = "chat-accept";
const string kChatDeny = "chat-denied";
const string kChatStop = "chat-closed";
const string kChatSignal = "chat-signal";
const string kStreamType = "stream-type";

P2PPeerConnectionChannel::P2PPeerConnectionChannel(const std::string& remote_id, SignalingSenderInterface* sender)
    :signaling_sender_(sender),
     remote_id_(remote_id),
     state_(kReady),
     peer_connection_(nullptr),
     factory_(PeerConnectionDependencyFactory::Get()),
     pc_thread_(new PeerConnectionThread),
     callback_thread_(new PeerConnectionThread) {
  CHECK(signaling_sender_);
}

void P2PPeerConnectionChannel::Invite(std::function<void()> success, std::function<void(int)> failure) {
  Json::Value json;
  json[kMessageTypeKey]=kChatInvitation;
  SendSignalingMessage(json, success, failure);
}

void P2PPeerConnectionChannel::OnIncomingMessage(const std::string& message) {
  LOG(LS_INFO)<<"OnIncomingMessage: "<<message;
  ASSERT(!message.empty());
  Json::Reader reader;
  Json::Value jsonMessage;
  if(!reader.parse(message, jsonMessage)){
    LOG(WARNING) << "Received unknown message";
    return;
  }
  std::string messageType;
  GetStringFromJsonObject(jsonMessage, kMessageTypeKey, &messageType);
  if(messageType.empty()){
    LOG(WARNING) << "Received unknown message";
    return;
  }
  if(messageType==kChatInvitation){
    OnMessageInvitation();
  }
  else if(messageType==kChatStop){
    OnMessageStop();
  }
  else if(messageType==kChatAccept){
    OnMessageAcceptance();
  }
  else{
    LOG(WARNING) << "Received unknown message";
    return;
  }
}

void P2PPeerConnectionChannel::ChangeState(State state) {
  state_=state;
}

void P2PPeerConnectionChannel::AddObserver(P2PPeerConnectionChannelObserver* observer) {
  observers_.push_back(observer);
}

void P2PPeerConnectionChannel::RemoveObserver(P2PPeerConnectionChannelObserver *observer) {
  observers_.erase(std::remove(observers_.begin(), observers_.end(), observer), observers_.end());
}

bool P2PPeerConnectionChannel::InitializePeerConnection() {
  DCHECK(!peer_connection_.get());
  CHECK(factory_.get());
  webrtc::PeerConnectionInterface::RTCConfiguration config;
  peer_connection_=(factory_->CreatePeerConnection(config, nullptr, this)).get();
  if(!peer_connection_.get()) {
    LOG(LS_ERROR) << "Failed to initialize PeerConnection.";
    return false;
  }
  return true;
}

void P2PPeerConnectionChannel::SendSignalingMessage(const Json::Value& data, std::function<void()> success, std::function<void(int)> failure) {
  CHECK(signaling_sender_);
  std::string jsonString=JsonValueToString(data);
  signaling_sender_->Send(jsonString, remote_id_, success, failure);
}

void P2PPeerConnectionChannel::OnMessageInvitation() {
  ChangeState(kPending);
  for (std::vector<P2PPeerConnectionChannelObserver*>::iterator it=observers_.begin(); it!=observers_.end(); it++){
    (*it)->OnInvited(remote_id_);
  }
}

void P2PPeerConnectionChannel::OnMessageAcceptance() {
  ChangeState(kConnecting);
  for (std::vector<P2PPeerConnectionChannelObserver*>::iterator it=observers_.begin(); it!=observers_.end(); it++){
    (*it)->OnAccepted(remote_id_);
  }
}

void P2PPeerConnectionChannel::OnMessageStop() {
}

void P2PPeerConnectionChannel::OnSignalingChange(PeerConnectionInterface::SignalingState new_state) {
  // TODO:
}

void P2PPeerConnectionChannel::OnAddStream(MediaStreamInterface* stream) {
  // TODO:
}

void P2PPeerConnectionChannel::OnRemoveStream(MediaStreamInterface* stream) {
  // TODO:
}

void P2PPeerConnectionChannel::OnDataChannel(webrtc::DataChannelInterface* data_channel) {
  // TODO:
}

void P2PPeerConnectionChannel::OnRenegotiationNeeded() {
  // TODO:
}

void P2PPeerConnectionChannel::OnIceConnectionChange(PeerConnectionInterface::IceConnectionState new_state) {
  // TODO:
}

void P2PPeerConnectionChannel::OnIceGatheringChange(PeerConnectionInterface::IceGatheringState new_state) {
  // TODO:
}

void P2PPeerConnectionChannel::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
  // TODO:
}
}
