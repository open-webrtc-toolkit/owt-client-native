/*
 * Intel License
 */

#include <vector>
#include "webrtc/base/logging.h"
#include "talk/woogeen/sdk/base/functionalobserver.h"
#include "talk/woogeen/sdk/conference/conferencepeerconnectionchannel.h"

namespace woogeen {

using std::string;

enum ConferencePeerConnectionChannel::SessionState : int {
  kSessionStateReady = 1,  // Indicate the channel is ready. This is the initial state.
  kSessionStateOffered,  // Indicates local client has sent an invitation and waiting for an acceptance.
  kSessionStatePending,  // Indicates local client received an invitation and waiting for user's response.
  kSessionStateMatched,  // Indicates both sides agreed to start a WebRTC session. One of them will send an offer soon.
  kSessionStateConnecting,  // Indicates both sides are trying to connect to the other side.
  kSessionStateConnected,  // Indicates PeerConnection has been established.
};

enum ConferencePeerConnectionChannel::NegotiationState : int {
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

ConferencePeerConnectionChannel::ConferencePeerConnectionChannel(std::shared_ptr<ConferenceSignalingChannelInterface> signaling_channel)
    :signaling_channel_(signaling_channel),
     session_state_(kSessionStateReady),
     negotiation_state_(kNegotiationStateNone),
     callback_thread_(new PeerConnectionThread) {
  callback_thread_->Start();
  CHECK(signaling_channel_);
}

void ConferencePeerConnectionChannel::ChangeSessionState(SessionState state) {
  LOG(LS_INFO) << "PeerConnectionChannel change session state : " << state;
  session_state_ = state;
}

void ConferencePeerConnectionChannel::ChangeNegotiationState(NegotiationState state) {
  LOG(LS_INFO) << "PeerConnectionChannel change negotiation state : " << state;
  negotiation_state_ = state;
}

void ConferencePeerConnectionChannel::AddObserver(ConferencePeerConnectionChannelObserver* observer) {
  observers_.push_back(observer);
}

void ConferencePeerConnectionChannel::RemoveObserver(ConferencePeerConnectionChannelObserver *observer) {
  observers_.erase(std::remove(observers_.begin(), observers_.end(), observer), observers_.end());
}

void ConferencePeerConnectionChannel::CreateOffer() {
  LOG(LS_INFO) << "Create offer.";
  scoped_refptr<FunctionalCreateSessionDescriptionObserver> observer=FunctionalCreateSessionDescriptionObserver::Create(std::bind(&ConferencePeerConnectionChannel::OnCreateSessionDescriptionSuccess, this, std::placeholders::_1), std::bind(&ConferencePeerConnectionChannel::OnCreateSessionDescriptionFailure, this, std::placeholders::_1));
  rtc::TypedMessageData<scoped_refptr<FunctionalCreateSessionDescriptionObserver>>* data = new rtc::TypedMessageData<scoped_refptr<FunctionalCreateSessionDescriptionObserver>>(observer);
  LOG(LS_INFO) << "Post create offer";
  pc_thread_->Post(this, kMessageTypeCreateOffer, data);
}

void ConferencePeerConnectionChannel::CreateAnswer() {
  LOG(LS_INFO) << "Create answer.";
  scoped_refptr<FunctionalCreateSessionDescriptionObserver> observer=FunctionalCreateSessionDescriptionObserver::Create(std::bind(&ConferencePeerConnectionChannel::OnCreateSessionDescriptionSuccess, this, std::placeholders::_1), std::bind(&ConferencePeerConnectionChannel::OnCreateSessionDescriptionFailure, this, std::placeholders::_1));
  rtc::TypedMessageData<scoped_refptr<FunctionalCreateSessionDescriptionObserver>>* message_observer = new rtc::TypedMessageData<scoped_refptr<FunctionalCreateSessionDescriptionObserver>>(observer);
  LOG(LS_INFO) << "Post create answer";
  pc_thread_->Post(this, kMessageTypeCreateAnswer, message_observer);
}

void ConferencePeerConnectionChannel::SendSignalingMessage(const Json::Value& data, std::function<void()> success, std::function<void(std::unique_ptr<ConferenceException>)> failure) {
}

void ConferencePeerConnectionChannel::OnSignalingChange(PeerConnectionInterface::SignalingState new_state) {
  LOG(LS_INFO) << "Signaling state changed: " << new_state;
}

void ConferencePeerConnectionChannel::OnAddStream(MediaStreamInterface* stream) {
}

void ConferencePeerConnectionChannel::OnRemoveStream(MediaStreamInterface* stream) {
}

void ConferencePeerConnectionChannel::OnDataChannel(webrtc::DataChannelInterface* data_channel) {
  // TODO:
}

void ConferencePeerConnectionChannel::OnRenegotiationNeeded() {
}

void ConferencePeerConnectionChannel::OnIceConnectionChange(PeerConnectionInterface::IceConnectionState new_state) {
  LOG(LS_INFO) << "Ice connection state changed: " << new_state;
}

void ConferencePeerConnectionChannel::OnIceGatheringChange(PeerConnectionInterface::IceGatheringState new_state) {
  LOG(LS_INFO) << "Ice gathering state changed: " << new_state;
}

void ConferencePeerConnectionChannel::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
  LOG(LS_INFO) << "On ice candidate";
}

void ConferencePeerConnectionChannel::OnCreateSessionDescriptionSuccess(webrtc::SessionDescriptionInterface* desc) {
  LOG(LS_INFO) << "Create sdp success.";
  scoped_refptr<FunctionalSetSessionDescriptionObserver> observer=FunctionalSetSessionDescriptionObserver::Create(
      std::bind(&ConferencePeerConnectionChannel::OnSetLocalSessionDescriptionSuccess, this),
      std::bind(&ConferencePeerConnectionChannel::OnSetLocalSessionDescriptionFailure, this, std::placeholders::_1));
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
}

void ConferencePeerConnectionChannel::OnCreateSessionDescriptionFailure(const std::string& error) {
  LOG(LS_INFO) << "Create sdp failed.";
}

void ConferencePeerConnectionChannel::OnSetLocalSessionDescriptionSuccess() {
  LOG(LS_INFO) << "Set local sdp success.";
}

void ConferencePeerConnectionChannel::OnSetLocalSessionDescriptionFailure(const std::string& error) {
  LOG(LS_INFO) << "Set local sdp failed.";
}

void ConferencePeerConnectionChannel::OnSetRemoteSessionDescriptionSuccess() {
  PeerConnectionChannel::OnSetRemoteSessionDescriptionSuccess();
}

void ConferencePeerConnectionChannel::OnSetRemoteSessionDescriptionFailure(const std::string& error) {
  LOG(LS_INFO) << "Set remote sdp failed.";
}

bool ConferencePeerConnectionChannel::CheckNullPointer(uintptr_t pointer, std::function<void(std::unique_ptr<ConferenceException>)>on_failure) {
  if(pointer)
    return true;
  if(on_failure!=nullptr) {
    std::unique_ptr<ConferenceException> e(new ConferenceException(ConferenceException::kUnkown, "Nullptr is not allowed."));
    on_failure(std::move(e));
  }
  return false;
}

void ConferencePeerConnectionChannel::Publish(std::shared_ptr<LocalStream> stream, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  LOG(LS_INFO) << "Publish a local stream.";
  if(!CheckNullPointer((uintptr_t)stream.get(), on_failure)){
    LOG(LS_INFO) << "Local stream cannot be nullptr.";
    return;
  }
}

void ConferencePeerConnectionChannel::Unpublish(std::shared_ptr<LocalStream> stream, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if(!CheckNullPointer((uintptr_t)stream.get(), on_failure)){
    LOG(LS_INFO) << "Local stream cannot be nullptr.";
    return;
  }
}

void ConferencePeerConnectionChannel::Stop(std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  LOG(LS_INFO) << "Stop session.";
}

}
