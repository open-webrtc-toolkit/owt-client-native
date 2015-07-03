/*
 * Intel License
 */

#include <vector>
#include "webrtc/base/logging.h"
#include "talk/woogeen/sdk/base/peerconnectionchannel.h"

namespace woogeen {

PeerConnectionChannel::PeerConnectionChannel(webrtc::PeerConnectionInterface::RTCConfiguration& configuration)
    : peer_connection_(nullptr),
      factory_(nullptr),
      pc_thread_(nullptr),
      configuration_(configuration) {
}

PeerConnectionChannel::~PeerConnectionChannel(){
  if(peer_connection_!=nullptr){
    peer_connection_->Close();
  }
  if (pc_thread_!=nullptr)
    delete pc_thread_;
}

bool PeerConnectionChannel::InitializePeerConnection(){
  if(factory_.get()==nullptr)
    factory_=PeerConnectionDependencyFactory::Get();
  media_constraints_.AddOptional(MediaConstraintsInterface::kEnableDtlsSrtp, true);
  media_constraints_.SetMandatory(webrtc::MediaConstraintsInterface::kOfferToReceiveAudio, true);
  media_constraints_.SetMandatory(webrtc::MediaConstraintsInterface::kOfferToReceiveVideo, true);
  peer_connection_=(factory_->CreatePeerConnection(configuration_, &media_constraints_, this)).get();
  if(!peer_connection_.get()) {
    LOG(LS_ERROR) << "Failed to initialize PeerConnection.";
    return false;
  }
  if(pc_thread_==nullptr) {
    pc_thread_=new rtc::Thread();
    pc_thread_->Start();
  }
  CHECK(peer_connection_);
  CHECK(pc_thread_);
  return true;
}

const webrtc::SessionDescriptionInterface* PeerConnectionChannel::LocalDescription(){
  CHECK(peer_connection_);
  return peer_connection_->local_description();
}

void PeerConnectionChannel::OnMessage(rtc::Message* msg) {
  LOG(LS_INFO) << "OnMessage";
  switch(msg->message_id) {
    case kMessageTypeClosePeerConnection:
      peer_connection_->Close();
      break;
    case kMessageTypeCreateOffer: {
      rtc::TypedMessageData<scoped_refptr<FunctionalCreateSessionDescriptionObserver>>* param = static_cast<rtc::TypedMessageData<scoped_refptr<FunctionalCreateSessionDescriptionObserver>>*>(msg->pdata);
      peer_connection_->CreateOffer(param->data(), &media_constraints_);
      delete param;
      break;
    }
    case kMessageTypeCreateAnswer: {
      rtc::TypedMessageData<scoped_refptr<FunctionalCreateSessionDescriptionObserver>>* param = static_cast<rtc::TypedMessageData<scoped_refptr<FunctionalCreateSessionDescriptionObserver>>*>(msg->pdata);
      peer_connection_->CreateAnswer(param->data(), nullptr);
      delete param;
      break;
    }
    case kMessageTypeSetLocalDescription: {
      SetSessionDescriptionMessage* param = static_cast<SetSessionDescriptionMessage*>(msg->pdata);
      peer_connection_->SetLocalDescription(param->observer, param->description);
      delete param;
      break;
    }
    case kMessageTypeSetRemoteDescription: {
      SetSessionDescriptionMessage* param = static_cast<SetSessionDescriptionMessage*>(msg->pdata);
      peer_connection_->SetRemoteDescription(param->observer, param->description);
      delete param;
      break;
    }
    case kMessageTypeSetRemoteIceCandidate: {
      rtc::TypedMessageData<webrtc::IceCandidateInterface*>* param = static_cast<rtc::TypedMessageData<webrtc::IceCandidateInterface*>*>(msg->pdata);
      peer_connection_->AddIceCandidate(param->data());
      delete param;
      break;
    }
    case kMessageTypeAddStream: {
      rtc::TypedMessageData<MediaStreamInterface*>* param = static_cast<rtc::TypedMessageData<MediaStreamInterface*>*>(msg->pdata);
      peer_connection_->AddStream(param->data());
      delete param;
      break;
    }
    case kMessageTypeRemoveStream: {
      rtc::TypedMessageData<MediaStreamInterface*>* param = static_cast<rtc::TypedMessageData<MediaStreamInterface*>*>(msg->pdata);
      peer_connection_->RemoveStream(param->data());
      delete param;
      break;
    }
    default:
      LOG(LS_WARNING) << "Unknown message type.";
  }
}

void PeerConnectionChannel::OnSetRemoteSessionDescriptionSuccess() {
  LOG(LS_INFO) << "Set remote sdp success.";
  if(peer_connection_->remote_description()&&peer_connection_->remote_description()->type() == "offer"){
    CreateAnswer();
  }
}

void PeerConnectionChannel::OnSetRemoteSessionDescriptionFailure(const std::string& error) {
  LOG(LS_INFO) << "Set remote sdp failed.";
}

void PeerConnectionChannel::OnCreateSessionDescriptionSuccess(webrtc::SessionDescriptionInterface* desc) {
  LOG(LS_INFO) << "Create sdp success.";
}

void PeerConnectionChannel::OnCreateSessionDescriptionFailure(const std::string& error) {
  LOG(LS_INFO) << "Create sdp failed.";
}

void PeerConnectionChannel::OnSetLocalSessionDescriptionSuccess() {
  LOG(LS_INFO) << "Set local sdp success.";
}

void PeerConnectionChannel::OnSetLocalSessionDescriptionFailure(const std::string& error) {
  LOG(LS_INFO) << "Set local sdp failed.";
}


void PeerConnectionChannel::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
}

void PeerConnectionChannel::OnSignalingChange(PeerConnectionInterface::SignalingState new_state){
}

void PeerConnectionChannel::OnAddStream(MediaStreamInterface* stream){
}

void PeerConnectionChannel::OnRemoveStream(MediaStreamInterface* stream){
}

void PeerConnectionChannel::OnDataChannel(webrtc::DataChannelInterface* data_channel){
}

void PeerConnectionChannel::OnRenegotiationNeeded(){
}

void PeerConnectionChannel::OnIceConnectionChange(PeerConnectionInterface::IceConnectionState new_state){
}

void PeerConnectionChannel::OnIceGatheringChange(PeerConnectionInterface::IceGatheringState new_state){
}

}
