/*
 * Intel License
 */

#include <vector>
#include <thread>
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

P2PPeerConnectionChannel::P2PPeerConnectionChannel(webrtc::PeerConnectionInterface::RTCConfiguration& configuration, const std::string& local_id, const std::string& remote_id, P2PSignalingSenderInterface* sender)
   : PeerConnectionChannel(configuration),
     signaling_sender_(sender),
     local_id_(local_id),
     remote_id_(remote_id),
     session_state_(kSessionStateReady),
     negotiation_state_(kNegotiationStateNone),
     negotiation_needed_(false),
     last_disconnect_(std::chrono::time_point<std::chrono::system_clock>::max()),
     callback_thread_(new PeerConnectionThread){
  callback_thread_->Start();
  CHECK(signaling_sender_);
}

void P2PPeerConnectionChannel::Invite(std::function<void()> success, std::function<void(std::unique_ptr<P2PException>)> failure) {
  SendStop(nullptr, nullptr);  // Just try to clean up remote side. No callback is needed.
  Json::Value json;
  json[kMessageTypeKey]=kChatInvitation;
  SendSignalingMessage(json, success, failure);
  ChangeSessionState(kSessionStateOffered);
}

void P2PPeerConnectionChannel::Accept(std::function<void()> on_success, std::function<void(std::unique_ptr<P2PException>)> on_failure) {
  if(session_state_!=kSessionStatePending)
    return;
  InitializePeerConnection();
  SendAcceptance(on_success, on_failure);
  ChangeSessionState(kSessionStateMatched);
}

void P2PPeerConnectionChannel::Deny(std::function<void()> on_success, std::function<void(std::unique_ptr<P2PException>)> on_failure) {
  if(session_state_!=kSessionStatePending)
    return;
  SendDeny(on_success, on_failure);
  ChangeSessionState(kSessionStateReady);
}

void P2PPeerConnectionChannel::OnIncomingSignalingMessage(const std::string& message) {
  LOG(LS_INFO)<<"OnIncomingMessage: "<<message;
  ASSERT(!message.empty());
  Json::Reader reader;
  Json::Value json_message;
  if(!reader.parse(message, json_message)){
    LOG(LS_WARNING) << "Cannot parse incoming message.";
    return;
  }
  std::string message_type;
  rtc::GetStringFromJsonObject(json_message, kMessageTypeKey, &message_type);
  if(message_type.empty()){
    LOG(LS_WARNING) << "Cannot get type from incoming message.";
    return;
  }
  if(message_type==kChatInvitation){
    OnMessageInvitation();
  }
  else if(message_type==kChatStop){
    OnMessageStop();
  }
  else if(message_type==kChatAccept){
    OnMessageAcceptance();
  }
  else if(message_type==kChatDeny){
    OnMessageDeny();
  }
  else if(message_type==kChatSignal){
    Json::Value signal;
    rtc::GetValueFromJsonObject(json_message,kMessageDataKey, &signal);
    OnMessageSignal(signal);
  }
  else if(message_type==kChatNegotiationAccepted){
    OnMessageNegotiationAcceptance();
  }
  else if(message_type==kChatNegotiationNeeded){
    OnMessageNegotiationNeeded();
  }
  else if(message_type==kStreamType){
    Json::Value type_info;
    rtc::GetValueFromJsonObject(json_message, kMessageDataKey, &type_info);
    OnMessageStreamType(type_info);
  }
  else{
    LOG(LS_WARNING) << "Received unknown message type : "<<message_type;
    return;
  }
}

void P2PPeerConnectionChannel::ChangeSessionState(SessionState state) {
  LOG(LS_INFO) << "PeerConnectionChannel change session state : " << state;
  session_state_ = state;
}

void P2PPeerConnectionChannel::ChangeNegotiationState(NegotiationState state) {
  LOG(LS_INFO) << "PeerConnectionChannel change negotiation state : " << state;
  negotiation_state_ = state;
}

void P2PPeerConnectionChannel::AddObserver(P2PPeerConnectionChannelObserver* observer) {
  observers_.push_back(observer);
}

void P2PPeerConnectionChannel::RemoveObserver(P2PPeerConnectionChannelObserver *observer) {
  observers_.erase(std::remove(observers_.begin(), observers_.end(), observer), observers_.end());
}

void P2PPeerConnectionChannel::CreateOffer() {
  LOG(LS_INFO) << "Create offer.";
  scoped_refptr<FunctionalCreateSessionDescriptionObserver> observer=FunctionalCreateSessionDescriptionObserver::Create(std::bind(&P2PPeerConnectionChannel::OnCreateSessionDescriptionSuccess, this, std::placeholders::_1), std::bind(&P2PPeerConnectionChannel::OnCreateSessionDescriptionFailure, this, std::placeholders::_1));
  rtc::TypedMessageData<scoped_refptr<FunctionalCreateSessionDescriptionObserver>>* data = new rtc::TypedMessageData<scoped_refptr<FunctionalCreateSessionDescriptionObserver>>(observer);
  LOG(LS_INFO) << "Post create offer";
  pc_thread_->Post(this, kMessageTypeCreateOffer, data);
}

void P2PPeerConnectionChannel::CreateAnswer() {
  LOG(LS_INFO) << "Create answer.";
  scoped_refptr<FunctionalCreateSessionDescriptionObserver> observer=FunctionalCreateSessionDescriptionObserver::Create(std::bind(&P2PPeerConnectionChannel::OnCreateSessionDescriptionSuccess, this, std::placeholders::_1), std::bind(&P2PPeerConnectionChannel::OnCreateSessionDescriptionFailure, this, std::placeholders::_1));
  rtc::TypedMessageData<scoped_refptr<FunctionalCreateSessionDescriptionObserver>>* message_observer = new rtc::TypedMessageData<scoped_refptr<FunctionalCreateSessionDescriptionObserver>>(observer);
  LOG(LS_INFO) << "Post create answer";
  pc_thread_->Post(this, kMessageTypeCreateAnswer, message_observer);
}

void P2PPeerConnectionChannel::SendNegotiationAccepted(){
  Json::Value json;
  json[kMessageTypeKey]=kChatNegotiationAccepted;
  SendSignalingMessage(json, nullptr, nullptr);
  ChangeNegotiationState(kNegotiationStateAccepted);
}

void P2PPeerConnectionChannel::SendSignalingMessage(const Json::Value& data, std::function<void()> success, std::function<void(std::unique_ptr<P2PException>)> failure) {
  CHECK(signaling_sender_);
  std::string jsonString=rtc::JsonValueToString(data);
  signaling_sender_->SendSignalingMessage(jsonString, remote_id_, success, [=](int){
    if(failure==nullptr)
      return;
    std::unique_ptr<P2PException> e(new P2PException(P2PException::kClientInvalidArgument, "Send signaling message failed."));
    failure(std::move(e));
  });
}

void P2PPeerConnectionChannel::OnMessageInvitation() {
  switch(session_state_) {
    case kSessionStateReady:
    case kSessionStatePending:
      ChangeSessionState(kSessionStatePending);
      for (std::vector<P2PPeerConnectionChannelObserver*>::iterator it=observers_.begin(); it!=observers_.end(); it++){
        (*it)->OnInvited(remote_id_);
      }
      break;
    case kSessionStateOffered:
      if(remote_id_.compare(local_id_)>0){
        SendAcceptance(nullptr, nullptr);
        ChangeSessionState(kSessionStateMatched);
      }
      break;
    default:
      LOG(LS_INFO) << "Ignore invitation because already connected.";
  }
}

void P2PPeerConnectionChannel::OnMessageAcceptance() {
  LOG(LS_INFO) << "Remote user accepted invitation.";
  if(session_state_!=kSessionStateOffered&&session_state_!=kSessionStateMatched)
    return;
  ChangeSessionState(kSessionStateMatched);
  for (std::vector<P2PPeerConnectionChannelObserver*>::iterator it=observers_.begin(); it!=observers_.end(); ++it){
    (*it)->OnAccepted(remote_id_);
  }
  InitializePeerConnection();
  CreateOffer();
}

void P2PPeerConnectionChannel::OnMessageStop() {
  switch(session_state_){
    case kSessionStateConnecting:
    case kSessionStateConnected:
      pc_thread_->Send(this, kMessageTypeClosePeerConnection, nullptr);
    case kSessionStateMatched:
      ChangeSessionState(kSessionStateReady);
      break;
    default:
      LOG(LS_WARNING) << "Received stop event on unexpected state. Current state: " << session_state_;
  }
}

void P2PPeerConnectionChannel::OnMessageDeny() {
  LOG(LS_INFO) << "Remote user denied invitation";
  for (std::vector<P2PPeerConnectionChannelObserver*>::iterator it=observers_.begin(); it!=observers_.end(); ++it){
    (*it)->OnDenied(remote_id_);
  }
  ChangeSessionState(kSessionStateReady);
}

void P2PPeerConnectionChannel::OnMessageNegotiationNeeded() {
  LOG(LS_INFO) << "Received negotiation needed event, current negotiation state:" << negotiation_state_;
  if(negotiation_state_==kNegotiationStateNone||(negotiation_state_==kNegotiationStateSent&&local_id_.compare(remote_id_)<0)){
    ChangeNegotiationState(kNegotiationStateAccepted);
    negotiation_needed_=true;
    SendNegotiationAccepted();
  }
}

void P2PPeerConnectionChannel::OnMessageNegotiationAcceptance() {
  LOG(LS_INFO) << "Post create offer";
  CreateOffer();
}

void P2PPeerConnectionChannel::OnMessageSignal(Json::Value& message) {
  string type;
  string desc;
  rtc::GetStringFromJsonObject(message, kSessionDescriptionTypeKey, &type);
  if(type=="offer"||type=="answer"){
    string sdp;
    if(!rtc::GetStringFromJsonObject(message, kSessionDescriptionSdpKey, &sdp)) {
      LOG(LS_WARNING) << "Cannot parse received sdp.";
      return;
    }
    webrtc::SessionDescriptionInterface* desc(
        webrtc::CreateSessionDescription(type, sdp, nullptr));
    if(!desc){
      LOG(LS_ERROR) << "Failed to create session description.";
      return;
    }
    scoped_refptr<FunctionalSetSessionDescriptionObserver> observer=FunctionalSetSessionDescriptionObserver::Create(
      std::bind(&P2PPeerConnectionChannel::OnSetRemoteSessionDescriptionSuccess, this),
      std::bind(&P2PPeerConnectionChannel::OnSetRemoteSessionDescriptionFailure, this, std::placeholders::_1));
    SetSessionDescriptionMessage* msg = new SetSessionDescriptionMessage(observer.get(), desc);
    LOG(LS_INFO) << "Post set remote desc";
    pc_thread_->Post(this, kMessageTypeSetRemoteDescription, msg);
  } else if (type=="candidates"){
    string sdp_mid;
    string candidate;
    int sdp_mline_index;
    rtc::GetStringFromJsonObject(message, kIceCandidateSdpMidKey, &sdp_mid);
    rtc::GetStringFromJsonObject(message, kIceCandidateSdpNameKey, &candidate);
    rtc::GetIntFromJsonObject(message, kIceCandidateSdpMLineIndexKey, &sdp_mline_index);
    webrtc::IceCandidateInterface *ice_candidate = webrtc::CreateIceCandidate(sdp_mid, sdp_mline_index, candidate, nullptr);
    rtc::TypedMessageData<webrtc::IceCandidateInterface*>* param = new rtc::TypedMessageData<webrtc::IceCandidateInterface*>(ice_candidate);
    pc_thread_->Post(this, kMessageTypeSetRemoteIceCandidate, param);
  }
}

void P2PPeerConnectionChannel::OnMessageStreamType(Json::Value& stream_info) {
  string id;
  string type;
  rtc::GetStringFromJsonObject(stream_info, kStreamIdKey, &id);
  rtc::GetStringFromJsonObject(stream_info, kStreamTypeKey, &type);
  std::pair<std::string, std::string> stream_type_element;
  stream_type_element=std::make_pair(id, type);
  remote_stream_type_.insert(stream_type_element);
}

void P2PPeerConnectionChannel::OnSignalingChange(PeerConnectionInterface::SignalingState new_state) {
  LOG(LS_INFO) << "Signaling state changed: " << new_state;
  switch(new_state){
    case PeerConnectionInterface::SignalingState::kStable:
      if(!pending_publish_streams_.empty()||pending_unpublish_streams_.empty())
        DrainPendingStreams();
      ChangeNegotiationState(kNegotiationStateNone);
      break;
    default:
      break;
  }
}

void P2PPeerConnectionChannel::OnAddStream(MediaStreamInterface* stream) {
  if(remote_stream_type_.find(stream->label())==remote_stream_type_.end())  // This stream is invalid.
    return;
  std::shared_ptr<woogeen::RemoteCameraStream> remote_stream(new woogeen::RemoteCameraStream(stream, remote_id_));
  for (std::vector<P2PPeerConnectionChannelObserver*>::iterator it=observers_.begin(); it!=observers_.end(); ++it){
    (*it)->OnStreamAdded(remote_stream);
  }
}

void P2PPeerConnectionChannel::OnRemoveStream(MediaStreamInterface* stream) {
  if(remote_stream_type_.find(stream->label())==remote_stream_type_.end())  // This stream is invalid.
    return;
  std::shared_ptr<woogeen::RemoteCameraStream> remote_stream(new woogeen::RemoteCameraStream(stream, remote_id_));
  for (std::vector<P2PPeerConnectionChannelObserver*>::iterator it=observers_.begin(); it!=observers_.end(); ++it){
    (*it)->OnStreamRemoved(remote_stream);
  }
}

void P2PPeerConnectionChannel::OnDataChannel(webrtc::DataChannelInterface* data_channel) {
  // TODO:
}

void P2PPeerConnectionChannel::OnRenegotiationNeeded() {
  LOG(LS_INFO) << "On negotiation needed.";
  Json::Value json;
  json[kMessageTypeKey] = kChatNegotiationNeeded;
  SendSignalingMessage(json, nullptr, nullptr);
}

void P2PPeerConnectionChannel::OnIceConnectionChange(PeerConnectionInterface::IceConnectionState new_state) {
  LOG(LS_INFO) << "Ice connection state changed: " << new_state;
  switch(new_state){
    case webrtc::PeerConnectionInterface::kIceConnectionConnected:
    case webrtc::PeerConnectionInterface::kIceConnectionCompleted:
      if(session_state_==kSessionStateMatched){
        for (std::vector<P2PPeerConnectionChannelObserver*>::iterator it=observers_.begin(); it!=observers_.end(); it++){
          (*it)->OnStarted(remote_id_);
        }
      }
      ChangeSessionState(kSessionStateConnected);
      CheckWaitedList();
      // reset |last_disconnect_|
      last_disconnect_=std::chrono::time_point<std::chrono::system_clock>::max();
      break;
    case webrtc::PeerConnectionInterface::kIceConnectionDisconnected:
      last_disconnect_=std::chrono::system_clock::now();
      // TODO(jianjun): Check state after a few seconds.
      break;
    case webrtc::PeerConnectionInterface::kIceConnectionClosed:
      for (std::vector<P2PPeerConnectionChannelObserver*>::iterator it=observers_.begin(); it!=observers_.end(); it++){
        (*it)->OnStopped(remote_id_);
      }
      Stop(nullptr, nullptr);
      break;
    default:
      break;
  }
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
  LOG(LS_INFO) << "Create sdp success.";
  scoped_refptr<FunctionalSetSessionDescriptionObserver> observer=FunctionalSetSessionDescriptionObserver::Create(
      std::bind(&P2PPeerConnectionChannel::OnSetLocalSessionDescriptionSuccess, this),
      std::bind(&P2PPeerConnectionChannel::OnSetLocalSessionDescriptionFailure, this, std::placeholders::_1));
  SetSessionDescriptionMessage* msg = new SetSessionDescriptionMessage(observer.get(), desc);
  LOG(LS_INFO) << "Post set local desc";
  pc_thread_->Post(this, kMessageTypeSetLocalDescription, msg);
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
  LOG(LS_INFO) << "Create sdp failed.";
  Stop(nullptr, nullptr);
}

void P2PPeerConnectionChannel::OnSetLocalSessionDescriptionSuccess() {
  LOG(LS_INFO) << "Set local sdp success.";
}

void P2PPeerConnectionChannel::OnSetLocalSessionDescriptionFailure(const std::string& error) {
  LOG(LS_INFO) << "Set local sdp failed.";
  Stop(nullptr, nullptr);
}

void P2PPeerConnectionChannel::OnSetRemoteSessionDescriptionSuccess() {
  PeerConnectionChannel::OnSetRemoteSessionDescriptionSuccess();
}

void P2PPeerConnectionChannel::OnSetRemoteSessionDescriptionFailure(const std::string& error) {
  LOG(LS_INFO) << "Set remote sdp failed.";
  Stop(nullptr, nullptr);
}

bool P2PPeerConnectionChannel::CheckNullPointer(uintptr_t pointer, std::function<void(std::unique_ptr<P2PException>)>on_failure) {
  if(pointer)
    return true;
  if(on_failure!=nullptr) {
    std::unique_ptr<P2PException> e(new P2PException(P2PException::kClientInvalidArgument, "Nullptr is not allowed."));
    on_failure(std::move(e));
  }
  return false;
}

void P2PPeerConnectionChannel::Publish(std::shared_ptr<LocalStream> stream, std::function<void()> on_success, std::function<void(std::unique_ptr<P2PException>)> on_failure) {
  LOG(LS_INFO) << "Publish a local stream.";
  if(!CheckNullPointer((uintptr_t)stream.get(), on_failure)){
    LOG(LS_INFO) << "Local stream cannot be nullptr.";
    return;
  }
  pending_publish_streams_mutex_.lock();
  pending_publish_streams_.push_back(stream);
  pending_publish_streams_mutex_.unlock();
  if(on_success){
    std::thread t(on_success);
    t.detach();
  }
  LOG(LS_INFO) << "Session state: "<<session_state_;
  LOG(LS_INFO) << "Negotiation state: "<<negotiation_state_;
  if(session_state_==SessionState::kSessionStateConnected&&negotiation_state_==NegotiationState::kNegotiationStateNone)
    DrainPendingStreams();
}

void P2PPeerConnectionChannel::Unpublish(std::shared_ptr<LocalStream> stream, std::function<void()> on_success, std::function<void(std::unique_ptr<P2PException>)> on_failure) {
  if(!CheckNullPointer((uintptr_t)stream.get(), on_failure)){
    LOG(LS_INFO) << "Local stream cannot be nullptr.";
    return;
  }
  pending_unpublish_streams_mutex_.lock();
  pending_unpublish_streams_.push_back(stream);
  pending_unpublish_streams_mutex_.unlock();
  if(on_success){
    std::thread t(on_success);
    t.detach();
  }
  if(session_state_==SessionState::kSessionStateConnected&&negotiation_state_==NegotiationState::kNegotiationStateNone)
    DrainPendingStreams();
}

void P2PPeerConnectionChannel::Stop(std::function<void()> on_success, std::function<void(std::unique_ptr<P2PException>)> on_failure) {
  LOG(LS_INFO) << "Stop session.";
  switch(session_state_){
    case kSessionStateConnecting:
    case kSessionStateConnected:
      pc_thread_->Post(this, kMessageTypeClosePeerConnection, nullptr);
    case kSessionStateMatched:
      SendStop(on_success, on_failure);
      ChangeSessionState(kSessionStateReady);
      break;
    default:
      if(on_failure!=nullptr) {
        std::unique_ptr<P2PException> e(new P2PException(P2PException::kClientInvalidState, "Cannot stop a session haven't started."));
        on_failure(std::move(e));
      }
      return;
  }
  if(on_success!=nullptr){
    std::thread t(on_success);
    t.detach();
  }
}

void P2PPeerConnectionChannel::DrainPendingStreams() {
  LOG(LS_INFO) << "Draining pending stream";
  pending_publish_streams_mutex_.lock();
  for (auto it = pending_publish_streams_.begin(); it != pending_publish_streams_.end(); ++it) {
    std::shared_ptr<LocalStream> stream = *it;
    scoped_refptr<webrtc::MediaStreamInterface> media_stream = stream->MediaStream();
    Json::Value json;
    json[kMessageTypeKey] = kStreamType;
    Json::Value stream_info;
    stream_info[kStreamIdKey] = media_stream->label();
    stream_info[kStreamTypeKey] = "video";
    json[kMessageDataKey] = stream_info;
    SendSignalingMessage(json, nullptr, nullptr);
    rtc::TypedMessageData<MediaStreamInterface*>* param = new rtc::TypedMessageData<MediaStreamInterface*>(media_stream);
    LOG(LS_INFO) << "Post add stream";
    pc_thread_->Post(this, kMessageTypeAddStream, param);
  }
  pending_publish_streams_.clear();
  pending_publish_streams_mutex_.unlock();
  pending_unpublish_streams_mutex_.lock();
  LOG(LS_INFO) << "Get unpublish stream lock.";
  for (auto it = pending_unpublish_streams_.begin(); it != pending_unpublish_streams_.end(); ++it) {
    LOG(LS_INFO) << "Remove a stream from peer connection.";
    std::shared_ptr<LocalStream> stream = *it;
    scoped_refptr<webrtc::MediaStreamInterface> media_stream = stream->MediaStream();
    rtc::TypedMessageData<MediaStreamInterface*>* param = new rtc::TypedMessageData<MediaStreamInterface*>(media_stream);
    LOG(LS_INFO) << "Post remove stream";
    pc_thread_->Post(this, kMessageTypeRemoveStream, param);
  }
  pending_unpublish_streams_.clear();
  pending_unpublish_streams_mutex_.unlock();
}

void P2PPeerConnectionChannel::SendAcceptance(std::function<void()> on_success, std::function<void(std::unique_ptr<P2PException>)> on_failure){
  Json::Value json;
  json[kMessageTypeKey]=kChatAccept;
  SendSignalingMessage(json, on_success, on_failure);
}

void P2PPeerConnectionChannel::SendStop(std::function<void()> on_success, std::function<void(std::unique_ptr<P2PException>)> on_failure){
  LOG(LS_INFO) << "Send stop.";
  Json::Value json;
  json[kMessageTypeKey]=kChatStop;
  SendSignalingMessage(json, on_success, on_failure);
}

void P2PPeerConnectionChannel::SendDeny(std::function<void()> on_success, std::function<void(std::unique_ptr<P2PException>)> on_failure){
  Json::Value json;
  json[kMessageTypeKey]=kChatDeny;
  SendSignalingMessage(json, on_success, on_failure);
}

void P2PPeerConnectionChannel::ClosePeerConnection() {
  LOG(LS_INFO) << "Close peer connection.";
  CHECK(pc_thread_);
  pc_thread_->Send(this, kMessageTypeClosePeerConnection, nullptr);
  ChangeSessionState(kSessionStateReady);
}

void P2PPeerConnectionChannel::CheckWaitedList() {
  if(!pending_publish_streams_.empty() || !pending_unpublish_streams_.empty()) {
    DrainPendingStreams();
  }
  else if(negotiation_needed_){
    SendNegotiationAccepted();
  }
}


}
