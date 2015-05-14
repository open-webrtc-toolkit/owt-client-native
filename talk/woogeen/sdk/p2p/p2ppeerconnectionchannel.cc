/*
 * Intel License
 */

#include <vector>
#include "webrtc/base/logging.h"
#include "talk/woogeen/sdk/p2p/p2ppeerconnectionchannel.h"
#include "talk/woogeen/sdk/base/functionalobserver.h"

namespace woogeen {

using std::string;

enum P2PPeerConnectionChannel::SessionState : int {
  kSessionStateReady = 1,  // Indicate the channel is ready. This is the initial state.
  kSessionStateOffered,  // Indicates local client has sent an invitation and waiting for an acceptance.
  kSessionStatePending,  // Indicates local client received an invitation and waiting for user's response.
  kSessionStateMatched,  // Indicates both sides agreed to start a WebRTC session. One of them will send an offer soon.
  kSessionStateConnecting,  // Indicates both sides are trying to connect to the other side.
  kSessionStateConnected,  // Indicates PeerConnection has been established.
};

enum P2PPeerConnectionChannel::NegotiationState : int {
  kNegotiationStateNone = 1,  // Indicates not in renegotiation.
  kNegotiationStateSent,  // Indicates a negotiation request has been sent to remote user.
  kNegotiationStateReceived,  // Indicates local side has received a negotiation request from remote user.
  kNegotiationStateAccepted,  // Indicates local side has accepted remote user's negotiation request.
};

// Signaling message type
const string kMessageTypeKey = "type";
const string kMessageDataKey = "data";
const string kChatInvitation = "chat-invitation";
const string kChatAccept = "chat-accepted";
const string kChatDeny = "chat-denied";
const string kChatStop = "chat-closed";
const string kChatSignal = "chat-signal";
const string kChatNegotiationNeeded = "chat-negotiation-needed";
const string kChatNegotiationAccepted = "chat-negotiation-accepted";
const string kStreamType = "stream-type";

// Stream type member key
const string kStreamIdKey = "streamId";
const string kStreamTypeKey = "type";

// Session description member key
const string kSessionDescriptionTypeKey = "type";
const string kSessionDescriptionSdpKey = "sdp";

// ICE candidate member key
const string kIceCandidateSdpMidKey = "sdpMid";
const string kIceCandidateSdpMLineIndexKey = "sdpMLineIndex";
const string kIceCandidateSdpNameKey = "candidate";

P2PPeerConnectionChannel::P2PPeerConnectionChannel(const std::string& remote_id, SignalingSenderInterface* sender)
    :signaling_sender_(sender),
     remote_id_(remote_id),
     session_state_(kSessionStateReady),
     peer_connection_(nullptr),
     factory_(nullptr),
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
    LOG(LS_WARNING) << "Received unknown message";
    return;
  }
  std::string messageType;
  GetStringFromJsonObject(jsonMessage, kMessageTypeKey, &messageType);
  if(messageType.empty()){
    LOG(LS_WARNING) << "Received unknown message";
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
  else if(messageType==kChatSignal){
    Json::Value signal;
    GetValueFromJsonObject(jsonMessage,kMessageDataKey, &signal);
    OnMessageSignal(signal);
  }
  else if(messageType==kChatNegotiationAccepted){
    OnMessageNegotiationAcceptance();
  }
  else if(messageType==kChatNegotiationNeeded){
    OnMessageNegotiationNeeded();
  }
  else{
    LOG(LS_WARNING) << "Received unknown message type "<<messageType;
    return;
  }
}

void P2PPeerConnectionChannel::ChangeSessionState(SessionState state) {
  session_state_=state;
}

void P2PPeerConnectionChannel::AddObserver(P2PPeerConnectionChannelObserver* observer) {
  observers_.push_back(observer);
}

void P2PPeerConnectionChannel::RemoveObserver(P2PPeerConnectionChannelObserver *observer) {
  observers_.erase(std::remove(observers_.begin(), observers_.end(), observer), observers_.end());
}

bool P2PPeerConnectionChannel::InitializePeerConnection() {
  DCHECK(!peer_connection_.get());
  if(factory_.get()==nullptr)
    factory_=PeerConnectionDependencyFactory::Get();
  webrtc::PeerConnectionInterface::RTCConfiguration config;
  media_constraints_.AddOptional(MediaConstraintsInterface::kEnableDtlsSrtp, true);
  peer_connection_=(factory_->CreatePeerConnection(config, &media_constraints_, this)).get();
  if(!peer_connection_.get()) {
    LOG(LS_ERROR) << "Failed to initialize PeerConnection.";
    return false;
  }
  pc_thread_=new woogeen::PeerConnectionThread();
  callback_thread_=new woogeen::PeerConnectionThread();
  pc_thread_->Start();
  callback_thread_->Start();
  return true;
}

void P2PPeerConnectionChannel::CreateOffer() {
  LOG(LS_INFO) << "Create offer";
  scoped_refptr<FunctionalCreateSessionDescriptionObserver> observer=FunctionalCreateSessionDescriptionObserver::Create(std::bind(&P2PPeerConnectionChannel::OnCreateSessionDescriptionSuccess, this, std::placeholders::_1), std::bind(&P2PPeerConnectionChannel::OnCreateSessionDescriptionFailure, this, std::placeholders::_1));
  media_constraints_.SetMandatory(webrtc::MediaConstraintsInterface::kOfferToReceiveAudio, true);
  media_constraints_.SetMandatory(webrtc::MediaConstraintsInterface::kOfferToReceiveVideo, true);
  peer_connection_->CreateOffer(observer, &media_constraints_);
  // pc_thread_->Invoke<int>(Bind(&PeerConnectionInterface::CreateOffer, peer_connection_.get(), observer));
}

void P2PPeerConnectionChannel::SendSignalingMessage(const Json::Value& data, std::function<void()> success, std::function<void(int)> failure) {
  CHECK(signaling_sender_);
  std::string jsonString=JsonValueToString(data);
  signaling_sender_->Send(jsonString, remote_id_, success, failure);
}

void P2PPeerConnectionChannel::OnMessageInvitation() {
  ChangeSessionState(kSessionStatePending);
  for (std::vector<P2PPeerConnectionChannelObserver*>::iterator it=observers_.begin(); it!=observers_.end(); it++){
    (*it)->OnInvited(remote_id_);
  }
}

void P2PPeerConnectionChannel::OnMessageAcceptance() {
  ChangeSessionState(kSessionStateConnecting);
  for (std::vector<P2PPeerConnectionChannelObserver*>::iterator it=observers_.begin(); it!=observers_.end(); it++){
    (*it)->OnAccepted(remote_id_);
  }
  InitializePeerConnection();
  pc_thread_->Invoke<void>(Bind(&P2PPeerConnectionChannel::CreateOffer, this));
}

void P2PPeerConnectionChannel::OnMessageStop() {
}

void P2PPeerConnectionChannel::OnMessageNegotiationNeeded() {
  // TODO: Check if local client ready to accept negotiation.
  Json::Value json;
  json[kMessageTypeKey]=kChatNegotiationAccepted;
  SendSignalingMessage(json, nullptr, nullptr);
}

void P2PPeerConnectionChannel::OnMessageNegotiationAcceptance() {
  pc_thread_->Invoke<void>(Bind(&P2PPeerConnectionChannel::CreateOffer, this));
}

void P2PPeerConnectionChannel::OnMessageSignal(Json::Value& message) {
  string type;
  string desc;
  GetStringFromJsonObject(message, kSessionDescriptionTypeKey, &type);
  if(type=="offer"||type=="answer"){
    string sdp;
    if(!GetStringFromJsonObject(message, kSessionDescriptionSdpKey, &sdp)) {
      LOG(LS_WARNING) << "Cannot parse received sdp.";
      return;
    }
    webrtc::SessionDescriptionInterface* desc(
        webrtc::CreateSessionDescription(type, sdp));
    if(!desc){
      LOG(LS_ERROR) << "Failed to create session description.";
      return;
    }
    scoped_refptr<FunctionalSetSessionDescriptionObserver> observer=FunctionalSetSessionDescriptionObserver::Create(
      std::bind(&P2PPeerConnectionChannel::OnSetSessionDescriptionSuccess, this),
      std::bind(&P2PPeerConnectionChannel::OnSetSessionDescriptionFailure, this, std::placeholders::_1));
    peer_connection_->SetRemoteDescription(observer, desc);
    // TODO:Create answer if it is an answer.
  }
}

void P2PPeerConnectionChannel::OnSignalingChange(PeerConnectionInterface::SignalingState new_state) {
  // TODO:
}

void P2PPeerConnectionChannel::OnAddStream(MediaStreamInterface* stream) {
  scoped_refptr<woogeen::RemoteStream> remote_stream = woogeen::RemoteStream::Create();
  remote_stream->MediaStream(stream);
  for (std::vector<P2PPeerConnectionChannelObserver*>::iterator it=observers_.begin(); it!=observers_.end(); it++){
    (*it)->OnStreamAdded(remote_id_, remote_stream);
  }
}

void P2PPeerConnectionChannel::OnRemoveStream(MediaStreamInterface* stream) {
  // TODO:
}

void P2PPeerConnectionChannel::OnDataChannel(webrtc::DataChannelInterface* data_channel) {
  // TODO:
}

void P2PPeerConnectionChannel::OnRenegotiationNeeded() {
  Json::Value json;
  json[kMessageTypeKey] = kChatNegotiationNeeded;
  SendSignalingMessage(json, nullptr, nullptr);
}

void P2PPeerConnectionChannel::OnIceConnectionChange(PeerConnectionInterface::IceConnectionState new_state) {
  LOG(LS_INFO) << "Ice connection state changed: " << new_state;
  // TOOD: set channel state
}

void P2PPeerConnectionChannel::OnIceGatheringChange(PeerConnectionInterface::IceGatheringState new_state) {
  LOG(LS_INFO) << "Ice gathering state changed: " << new_state;
}

void P2PPeerConnectionChannel::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
  LOG(LS_INFO) << "On ice candidate";
  Json::Value signal;
  signal[kSessionDescriptionTypeKey] = "candidates";
  signal[kIceCandidateSdpMLineIndexKey] = candidate->sdp_mline_index();
  signal[kIceCandidateSdpMidKey] = candidate->sdp_mid();
  string sdp;
  if(!candidate->ToString(&sdp)){
    LOG(LS_ERROR)<<"Failed to serialize candidate";
    return;
  }
  signal[kIceCandidateSdpNameKey] = sdp;
  Json::Value json;
  json[kMessageTypeKey]=kChatSignal;
  json[kMessageDataKey]=signal;
  SendSignalingMessage(json, nullptr, nullptr);
}

void P2PPeerConnectionChannel::OnCreateSessionDescriptionSuccess(webrtc::SessionDescriptionInterface* desc) {
  LOG(LS_INFO) << "Create sdp success: ";
  scoped_refptr<FunctionalSetSessionDescriptionObserver> observer=FunctionalSetSessionDescriptionObserver::Create(
      std::bind(&P2PPeerConnectionChannel::OnSetSessionDescriptionSuccess, this),
      std::bind(&P2PPeerConnectionChannel::OnSetSessionDescriptionFailure, this, std::placeholders::_1));
  peer_connection_->SetLocalDescription(observer, desc);
  string sdp;
  desc->ToString(&sdp);
  Json::Value signal;
  signal[kSessionDescriptionTypeKey] = desc->type();
  signal[kSessionDescriptionSdpKey] = sdp;
  Json::Value json;
  json[kMessageTypeKey]=kChatSignal;
  json[kMessageDataKey]=signal;
  SendSignalingMessage(json, nullptr, nullptr);
}

void P2PPeerConnectionChannel::OnCreateSessionDescriptionFailure(const std::string& error) {
  LOG(LS_INFO) << "Create sdp failed";
}

void P2PPeerConnectionChannel::OnSetSessionDescriptionSuccess() {
  LOG(LS_INFO) << "Set sdp success";
}

void P2PPeerConnectionChannel::OnSetSessionDescriptionFailure(const std::string& error) {
  LOG(LS_INFO) << "Set sdp failed";
}

void P2PPeerConnectionChannel::Publish(scoped_refptr<LocalStream> stream, std::function<void()> on_success, std::function<void(int)> on_failure) {
  scoped_refptr<webrtc::MediaStreamInterface> media_stream = stream->MediaStream();
  Json::Value json;
  json[kMessageTypeKey] = kStreamType;
  Json::Value stream_info;
  stream_info[kStreamIdKey] = media_stream->label();
  stream_info[kStreamTypeKey] = "video";
  json[kMessageDataKey] = stream_info;
  SendSignalingMessage(json, nullptr, nullptr);
  if(peer_connection_->AddStream(media_stream)){
    LOG(LS_ERROR)<<"Failed to add stream.";
    if(on_failure){
      on_failure(0);
    }
  }
}
}
