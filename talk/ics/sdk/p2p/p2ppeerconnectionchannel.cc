/*
 * Intel License
 */

#include <vector>
#include <thread>
#include "talk/ics/sdk/base/eventtrigger.h"
#include "talk/ics/sdk/base/functionalobserver.h"
#include "talk/ics/sdk/base/sysinfo.h"
#include "talk/ics/sdk/p2p/p2ppeerconnectionchannel.h"
#include "webrtc/rtc_base/logging.h"

namespace ics {
namespace p2p {

using std::string;

enum P2PPeerConnectionChannel::SessionState : int {
  kSessionStateReady =
      1,  // Indicate the channel is ready. This is the initial state.
  kSessionStateOffered,     // Indicates local client has sent an invitation and
                            // waiting for an acceptance.
  kSessionStatePending,     // Indicates local client received an invitation and
                            // waiting for user's response.
  kSessionStateMatched,     // Indicates both sides agreed to start a WebRTC
                            // session. One of them will send an offer soon.
  kSessionStateConnecting,  // Indicates both sides are trying to connect to the
                            // other side.
  kSessionStateConnected,   // Indicates PeerConnection has been established.
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

// UA member key
const string kUaKey = "ua";
const string kUaSdkKey = "sdk";
const string kUaSdkTypeKey = "type";
const string kUaSdkVersionKey = "version";
const string kUaRuntimeKey = "runtime";
const string kUaRuntimeNameKey = "name";
const string kUaRuntimeVersionKey = "version";

const string kDataChannelLabelForTextMessage = "message";

P2PPeerConnectionChannel::P2PPeerConnectionChannel(
    PeerConnectionChannelConfiguration configuration,
    const std::string& local_id,
    const std::string& remote_id,
    P2PSignalingSenderInterface* sender,
    std::shared_ptr<rtc::TaskQueue> event_queue)
    : PeerConnectionChannel(configuration),
      signaling_sender_(sender),
      local_id_(local_id),
      remote_id_(remote_id),
      is_caller_(false),
      session_state_(kSessionStateReady),
      negotiation_needed_(false),
      set_remote_sdp_task_(nullptr),
      last_disconnect_(
          std::chrono::time_point<std::chrono::system_clock>::max()),
      reconnect_timeout_(10),
      remote_side_supports_plan_b_(false),
      remote_side_supports_remove_stream_(false),
      is_creating_offer_(false),
      event_queue_(event_queue) {
  RTC_CHECK(signaling_sender_);
}

P2PPeerConnectionChannel::P2PPeerConnectionChannel(
    PeerConnectionChannelConfiguration configuration,
    const std::string& local_id,
    const std::string& remote_id,
    P2PSignalingSenderInterface* sender)
    : P2PPeerConnectionChannel(
          configuration,
          local_id,
          remote_id,
          sender,
          std::shared_ptr<rtc::TaskQueue>(
              new rtc::TaskQueue("PeerConnectionChannelEventQueue"))) {}

P2PPeerConnectionChannel::~P2PPeerConnectionChannel() {
  if (set_remote_sdp_task_)
    delete set_remote_sdp_task_;
  if (signaling_sender_)
    delete signaling_sender_;
}

Json::Value P2PPeerConnectionChannel::UaInfo() {
  Json::Value ua;
  Json::Value sdk;
  SysInfo sys_info = SysInfo::GetInstance();
  sdk[kUaSdkTypeKey] = sys_info.sdk.type;
  sdk[kUaSdkVersionKey] = sys_info.sdk.version;
  // Runtime values will be empty string on native SDK and will be browser info
  // on JavaScript SDK
  Json::Value runtime;
  runtime[kUaRuntimeNameKey] = sys_info.runtime.name;
  runtime[kUaRuntimeVersionKey] = sys_info.runtime.version;
  ua[kUaSdkKey] = sdk;
  ua[kUaRuntimeKey] = runtime;
  return ua;
}

void P2PPeerConnectionChannel::Invite(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<P2PException>)> on_failure) {
  if (session_state_ != kSessionStateReady &&
      session_state_ != kSessionStateOffered) {
    if (on_failure) {
      LOG(LS_WARNING) << "Cannot send invitation in this state: "
                      << session_state_;
      event_queue_->PostTask([on_failure] {
        std::unique_ptr<P2PException> e(
            new P2PException(P2PException::kClientInvalidState,
                             "Cannot send invitation in this state."));
        on_failure(std::move(e));
      });
    }
    return;
  }
  SendStop(
      nullptr,
      nullptr);  // Just try to clean up remote side. No callback is needed.
  Json::Value json;
  json[kMessageTypeKey] = kChatInvitation;
  Json::Value ua = UaInfo();
  json[kUaKey] = ua;
  SendSignalingMessage(json, on_success, on_failure);
  ChangeSessionState(kSessionStateOffered);
}

void P2PPeerConnectionChannel::Accept(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<P2PException>)> on_failure) {
  if (session_state_ != kSessionStatePending) {
    if (on_failure) {
      event_queue_->PostTask([on_failure] {
        std::unique_ptr<P2PException> e(
            new P2PException(P2PException::kClientInvalidState,
                             "Cannot accept invitation in this state."));
        on_failure(std::move(e));
      });
    }
    return;
  }
  is_caller_ = false;
  InitializePeerConnection();
  SendAcceptance(on_success, on_failure);
  ChangeSessionState(kSessionStateMatched);
  CreateDataChannel(kDataChannelLabelForTextMessage);
}

void P2PPeerConnectionChannel::Deny(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<P2PException>)> on_failure) {
  if (session_state_ != kSessionStatePending) {
    if (on_failure) {
      event_queue_->PostTask([on_failure] {
        std::unique_ptr<P2PException> e(
            new P2PException(P2PException::kClientInvalidState,
                             "Cannot deny invitation in this state."));
        on_failure(std::move(e));
      });
    }
    return;
  }
  SendDeny(on_success, on_failure);
  ChangeSessionState(kSessionStateReady);
}

void P2PPeerConnectionChannel::OnIncomingSignalingMessage(
    const std::string& message) {
  LOG(LS_INFO) << "OnIncomingMessage: " << message;
  RTC_DCHECK(!message.empty());
  Json::Reader reader;
  Json::Value json_message;
  if (!reader.parse(message, json_message)) {
    LOG(LS_WARNING) << "Cannot parse incoming message.";
    return;
  }
  std::string message_type;
  rtc::GetStringFromJsonObject(json_message, kMessageTypeKey, &message_type);
  if (message_type.empty()) {
    LOG(LS_WARNING) << "Cannot get type from incoming message.";
    return;
  }
  if (message_type == kChatInvitation) {
    Json::Value ua;
    rtc::GetValueFromJsonObject(json_message, kUaKey, &ua);
    OnMessageInvitation(ua);
  } else if (message_type == kChatStop) {
    OnMessageStop();
  } else if (message_type == kChatAccept) {
    Json::Value ua;
    rtc::GetValueFromJsonObject(json_message, kUaKey, &ua);
    OnMessageAcceptance(ua);
  } else if (message_type == kChatDeny) {
    OnMessageDeny();
  } else if (message_type == kChatSignal) {
    Json::Value signal;
    rtc::GetValueFromJsonObject(json_message, kMessageDataKey, &signal);
    OnMessageSignal(signal);
  } else if (message_type == kChatNegotiationNeeded) {
    OnMessageNegotiationNeeded();
  } else if (message_type == kStreamType) {
    Json::Value type_info;
    rtc::GetValueFromJsonObject(json_message, kMessageDataKey, &type_info);
    OnMessageStreamType(type_info);
  } else {
    LOG(LS_WARNING) << "Received unknown message type : " << message_type;
    return;
  }
}

void P2PPeerConnectionChannel::ChangeSessionState(SessionState state) {
  LOG(LS_INFO) << "PeerConnectionChannel change session state : " << state;
  session_state_ = state;
}

void P2PPeerConnectionChannel::AddObserver(
    P2PPeerConnectionChannelObserver* observer) {
  observers_.push_back(observer);
}

void P2PPeerConnectionChannel::RemoveObserver(
    P2PPeerConnectionChannelObserver* observer) {
  observers_.erase(std::remove(observers_.begin(), observers_.end(), observer),
                   observers_.end());
}

void P2PPeerConnectionChannel::CreateOffer() {
  {
    std::lock_guard<std::mutex> lock(is_creating_offer_mutex_);
    if (is_creating_offer_) {
      // Store creating offer request.
      negotiation_needed_ = true;
      return;
    } else {
      is_creating_offer_ = true;
    }
  }
  LOG(LS_INFO) << "Create offer.";
  negotiation_needed_ = false;
  scoped_refptr<FunctionalCreateSessionDescriptionObserver> observer =
      FunctionalCreateSessionDescriptionObserver::Create(
          std::bind(
              &P2PPeerConnectionChannel::OnCreateSessionDescriptionSuccess,
              this, std::placeholders::_1),
          std::bind(
              &P2PPeerConnectionChannel::OnCreateSessionDescriptionFailure,
              this, std::placeholders::_1));
  rtc::TypedMessageData<
      scoped_refptr<FunctionalCreateSessionDescriptionObserver>>* data =
      new rtc::TypedMessageData<
          scoped_refptr<FunctionalCreateSessionDescriptionObserver>>(observer);
  LOG(LS_INFO) << "Post create offer";
  pc_thread_->Post(RTC_FROM_HERE, this, kMessageTypeCreateOffer, data);
}

void P2PPeerConnectionChannel::CreateAnswer() {
  LOG(LS_INFO) << "Create answer.";
  scoped_refptr<FunctionalCreateSessionDescriptionObserver> observer =
      FunctionalCreateSessionDescriptionObserver::Create(
          std::bind(
              &P2PPeerConnectionChannel::OnCreateSessionDescriptionSuccess,
              this, std::placeholders::_1),
          std::bind(
              &P2PPeerConnectionChannel::OnCreateSessionDescriptionFailure,
              this, std::placeholders::_1));
  rtc::TypedMessageData<
      scoped_refptr<FunctionalCreateSessionDescriptionObserver>>*
      message_observer = new rtc::TypedMessageData<
          scoped_refptr<FunctionalCreateSessionDescriptionObserver>>(observer);
  LOG(LS_INFO) << "Post create answer";
  pc_thread_->Post(RTC_FROM_HERE, this, kMessageTypeCreateAnswer, message_observer);
}

void P2PPeerConnectionChannel::SendSignalingMessage(
    const Json::Value& data,
    std::function<void()> success,
    std::function<void(std::unique_ptr<P2PException>)> failure) {
  RTC_CHECK(signaling_sender_);
  std::string jsonString = rtc::JsonValueToString(data);
  signaling_sender_->SendSignalingMessage(
      jsonString, remote_id_, success, [=](int) {
        if (failure == nullptr)
          return;
        std::unique_ptr<P2PException> e(
            new P2PException(P2PException::kClientInvalidArgument,
                             "Send signaling message failed."));
        failure(std::move(e));
      });
}

void P2PPeerConnectionChannel::OnMessageInvitation(Json::Value& ua) {
  HandleRemoteCapability(ua);
  switch (session_state_) {
    case kSessionStateReady:
    case kSessionStatePending:
      ChangeSessionState(kSessionStatePending);
      for (std::vector<P2PPeerConnectionChannelObserver*>::iterator it =
               observers_.begin();
           it != observers_.end(); it++) {
        (*it)->OnInvited(remote_id_);
      }
      break;
    case kSessionStateOffered:
      if (remote_id_.compare(local_id_) > 0) {
        SendAcceptance(nullptr, nullptr);
        ChangeSessionState(kSessionStateMatched);
      }
      break;
    default:
      LOG(LS_INFO) << "Ignore invitation because already connected.";
  }
}

void P2PPeerConnectionChannel::OnMessageAcceptance(Json::Value& ua) {
  LOG(LS_INFO) << "Remote user accepted invitation.";
  if (session_state_ != kSessionStateOffered &&
      session_state_ != kSessionStateMatched)
    return;
  ChangeSessionState(kSessionStateMatched);
  for (std::vector<P2PPeerConnectionChannelObserver*>::iterator it =
           observers_.begin();
       it != observers_.end(); ++it) {
    (*it)->OnAccepted(remote_id_);
  }
  is_caller_ = true;
  HandleRemoteCapability(ua);
  InitializePeerConnection();
  ChangeSessionState(kSessionStateConnecting);
  CreateDataChannel(kDataChannelLabelForTextMessage);
}

void P2PPeerConnectionChannel::OnMessageStop() {
  switch (session_state_) {
    case kSessionStateConnecting:
    case kSessionStateConnected:
      pc_thread_->Send(RTC_FROM_HERE, this, kMessageTypeClosePeerConnection,
                       nullptr);
      ChangeSessionState(kSessionStateReady);
      break;
    case kSessionStatePending:
    case kSessionStateMatched:
      ChangeSessionState(kSessionStateReady);
      // Maybe we can add another event like "cancel" for such kind scenario.
      // For now, we trigger OnStop to indicate the invitation has been
      // canceled, and session is stopped.
      TriggerOnStopped();
      break;
    default:
      LOG(LS_WARNING)
          << "Received stop event on unexpected state. Current state: "
          << session_state_;
  }
}

void P2PPeerConnectionChannel::OnMessageDeny() {
  LOG(LS_INFO) << "Remote user denied invitation";
  for (std::vector<P2PPeerConnectionChannelObserver*>::iterator it =
           observers_.begin();
       it != observers_.end(); ++it) {
    (*it)->OnDenied(remote_id_);
  }
  ChangeSessionState(kSessionStateReady);
}

void P2PPeerConnectionChannel::OnMessageNegotiationNeeded() {
  LOG(LS_INFO) << "Received negotiation needed event";
  negotiation_needed_ = true;
  if (SignalingState() == PeerConnectionInterface::SignalingState::kStable) {
    CreateOffer();
  }
}

void P2PPeerConnectionChannel::OnMessageSignal(Json::Value& message) {
  if (session_state_ == kSessionStateReady ||
      session_state_ == kSessionStateOffered ||
      session_state_ == kSessionStatePending) {
    LOG(LS_WARNING)
        << "Received signaling message in invalid state. Current state: "
        << session_state_;
    return;
  }
  string type;
  string desc;
  rtc::GetStringFromJsonObject(message, kSessionDescriptionTypeKey, &type);
  if (type == "offer" || type == "answer") {
    if (type == "offer" && session_state_ == kSessionStateMatched) {
      ChangeSessionState(kSessionStateConnecting);
    }
    string sdp;
    if (!rtc::GetStringFromJsonObject(message, kSessionDescriptionSdpKey,
                                      &sdp)) {
      LOG(LS_WARNING) << "Cannot parse received sdp.";
      return;
    }
    webrtc::SessionDescriptionInterface* desc(
        webrtc::CreateSessionDescription(type, sdp, nullptr));
    if (!desc) {
      LOG(LS_ERROR) << "Failed to create session description.";
      return;
    }
    scoped_refptr<FunctionalSetSessionDescriptionObserver> observer =
        FunctionalSetSessionDescriptionObserver::Create(
            std::bind(
                &P2PPeerConnectionChannel::OnSetRemoteSessionDescriptionSuccess,
                this),
            std::bind(
                &P2PPeerConnectionChannel::OnSetRemoteSessionDescriptionFailure,
                this, std::placeholders::_1));
    SetSessionDescriptionMessage* msg =
        new SetSessionDescriptionMessage(observer.get(), desc);
    if (type == "offer" &&
        SignalingState() != webrtc::PeerConnectionInterface::kStable) {
      if (set_remote_sdp_task_) {
        delete set_remote_sdp_task_;
      }
      set_remote_sdp_task_ = msg;
    } else {
      LOG(LS_INFO) << "Post set remote desc";
      pc_thread_->Post(RTC_FROM_HERE, this, kMessageTypeSetRemoteDescription, msg);
    }
  } else if (type == "candidates") {
    string sdp_mid;
    string candidate;
    int sdp_mline_index;
    rtc::GetStringFromJsonObject(message, kIceCandidateSdpMidKey, &sdp_mid);
    rtc::GetStringFromJsonObject(message, kIceCandidateSdpNameKey, &candidate);
    rtc::GetIntFromJsonObject(message, kIceCandidateSdpMLineIndexKey,
                              &sdp_mline_index);
    webrtc::IceCandidateInterface* ice_candidate = webrtc::CreateIceCandidate(
        sdp_mid, sdp_mline_index, candidate, nullptr);
    rtc::TypedMessageData<webrtc::IceCandidateInterface*>* param =
        new rtc::TypedMessageData<webrtc::IceCandidateInterface*>(
            ice_candidate);
    pc_thread_->Post(RTC_FROM_HERE, this, kMessageTypeSetRemoteIceCandidate, param);
  }
}

void P2PPeerConnectionChannel::OnMessageStreamType(Json::Value& stream_info) {
  string id;
  string type;
  rtc::GetStringFromJsonObject(stream_info, kStreamIdKey, &id);
  rtc::GetStringFromJsonObject(stream_info, kStreamTypeKey, &type);
  std::pair<std::string, std::string> stream_type_element;
  stream_type_element = std::make_pair(id, type);
  remote_stream_type_.insert(stream_type_element);
}

void P2PPeerConnectionChannel::OnSignalingChange(
    PeerConnectionInterface::SignalingState new_state) {
  LOG(LS_INFO) << "Signaling state changed: " << new_state;
  switch (new_state) {
    case PeerConnectionInterface::SignalingState::kStable:
      if (set_remote_sdp_task_) {
        LOG(LS_INFO) << "Set stored remote description.";
        pc_thread_->Post(RTC_FROM_HERE, this, kMessageTypeSetRemoteDescription,
                         set_remote_sdp_task_);
        // Ownership will be transferred to message handler
        set_remote_sdp_task_ = nullptr;
      } else {
        CheckWaitedList();
      }
      break;
    default:
      break;
  }
}

void P2PPeerConnectionChannel::OnAddStream(
    rtc::scoped_refptr<MediaStreamInterface> stream) {
  LOG(LS_INFO) << "P2PPeerConnectionChannel::OnAddStream";
  if (remote_stream_type_.find(stream->label()) == remote_stream_type_.end()) {
    LOG(LS_WARNING) << "Missing stream type info for newly added stream.";
    RTC_DCHECK(false);
  }
  if (remote_stream_type_[stream->label()] == "screen") {
    std::shared_ptr<RemoteScreenStream> remote_stream(
        new RemoteScreenStream(stream, remote_id_));
    LOG(LS_INFO) << "Add screen stream";
    EventTrigger::OnEvent1<
        P2PPeerConnectionChannelObserver*,
        std::allocator<P2PPeerConnectionChannelObserver*>,
        void (ics::p2p::P2PPeerConnectionChannelObserver::*)(
            std::shared_ptr<RemoteScreenStream>),
        std::shared_ptr<RemoteScreenStream>>(
        observers_, event_queue_,
        &P2PPeerConnectionChannelObserver::OnStreamAdded, remote_stream);
    remote_streams_[stream->label()] = remote_stream;
  } else {
    LOG(LS_INFO) << "Add camera stream.";
    std::shared_ptr<RemoteCameraStream> remote_stream(
        new RemoteCameraStream(stream, remote_id_));
    EventTrigger::OnEvent1<
        P2PPeerConnectionChannelObserver*,
        std::allocator<P2PPeerConnectionChannelObserver*>,
        void (ics::p2p::P2PPeerConnectionChannelObserver::*)(
            std::shared_ptr<RemoteCameraStream>),
        std::shared_ptr<RemoteCameraStream>>(
        observers_, event_queue_,
        &P2PPeerConnectionChannelObserver::OnStreamAdded, remote_stream);
    remote_streams_[stream->label()] = remote_stream;
  }
}

void P2PPeerConnectionChannel::OnRemoveStream(
    rtc::scoped_refptr<MediaStreamInterface> stream) {
  if (remote_streams_.find(stream->label()) == remote_streams_.end() ||
      remote_stream_type_.find(stream->label()) == remote_stream_type_.end()) {
    LOG(LS_WARNING) << "Remove an invalid stream.";
    RTC_DCHECK(false);
    return;
  }
  if (remote_stream_type_[stream->label()] == "screen") {
    std::shared_ptr<RemoteScreenStream> remote_stream =
        std::static_pointer_cast<RemoteScreenStream>(
            remote_streams_[stream->label()]);
    EventTrigger::OnEvent1<
        P2PPeerConnectionChannelObserver*,
        std::allocator<P2PPeerConnectionChannelObserver*>,
        void (ics::p2p::P2PPeerConnectionChannelObserver::*)(
            std::shared_ptr<RemoteScreenStream>),
        std::shared_ptr<RemoteScreenStream>>(
        observers_, event_queue_,
        &P2PPeerConnectionChannelObserver::OnStreamRemoved, remote_stream);
  } else {
    std::shared_ptr<RemoteCameraStream> remote_stream =
        std::static_pointer_cast<RemoteCameraStream>(
            remote_streams_[stream->label()]);
    EventTrigger::OnEvent1<
        P2PPeerConnectionChannelObserver*,
        std::allocator<P2PPeerConnectionChannelObserver*>,
        void (ics::p2p::P2PPeerConnectionChannelObserver::*)(
            std::shared_ptr<RemoteCameraStream>),
        std::shared_ptr<RemoteCameraStream>>(
        observers_, event_queue_,
        &P2PPeerConnectionChannelObserver::OnStreamRemoved, remote_stream);
  }
  remote_streams_.erase(stream->label());
  remote_stream_type_.erase(stream->label());
}

void P2PPeerConnectionChannel::OnDataChannel(
    rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) {
  // If a new data channel is create, delete the old one to save resource.
  // Currently only one data channel for one connection. If we are going to
  // support multiple data channels(one for text, one for large files), replace
  // |data_channel_| with a map.
  if (data_channel_)
    data_channel_ = nullptr;
  data_channel_ = data_channel;
  data_channel_->RegisterObserver(this);
  DrainPendingMessages();
}

void P2PPeerConnectionChannel::OnRenegotiationNeeded() {
  LOG(LS_INFO) << "On negotiation needed.";
  if (!is_caller_) {
    if (session_state_ == kSessionStateConnecting ||
        session_state_ == kSessionStateConnected) {
      Json::Value json;
      json[kMessageTypeKey] = kChatNegotiationNeeded;
      SendSignalingMessage(json, nullptr, nullptr);
    }
    // If session is not connected, offer will be sent later. Nothing to do
    // here.
  } else if (SignalingState() ==
             PeerConnectionInterface::SignalingState::kStable) {
    CreateOffer();
  } else {
    negotiation_needed_ = true;
  }
}

void P2PPeerConnectionChannel::OnIceConnectionChange(
    PeerConnectionInterface::IceConnectionState new_state) {
  LOG(LS_INFO) << "Ice connection state changed: " << new_state;
  switch (new_state) {
    case webrtc::PeerConnectionInterface::kIceConnectionConnected:
    case webrtc::PeerConnectionInterface::kIceConnectionCompleted:
      if (session_state_ == kSessionStateConnecting) {
        for (std::vector<P2PPeerConnectionChannelObserver*>::iterator it =
                 observers_.begin();
             it != observers_.end(); it++) {
          (*it)->OnStarted(remote_id_);
        }
      }
      ChangeSessionState(kSessionStateConnected);
      CheckWaitedList();
      // reset |last_disconnect_|
      last_disconnect_ =
          std::chrono::time_point<std::chrono::system_clock>::max();
      break;
    case webrtc::PeerConnectionInterface::kIceConnectionDisconnected:
      last_disconnect_ = std::chrono::system_clock::now();
      // Check state after a period of time.
      std::thread([this]() {
        std::this_thread::sleep_for(std::chrono::seconds(reconnect_timeout_));
        if (std::chrono::system_clock::now() - last_disconnect_ >=
            std::chrono::seconds(reconnect_timeout_)) {
          LOG(LS_INFO) << "Detect reconnection failed, stop this session.";
          Stop(nullptr, nullptr);
        } else {
          LOG(LS_INFO) << "Detect reconnection succeed.";
        }
      }).detach();
      break;
    case webrtc::PeerConnectionInterface::kIceConnectionClosed:
      TriggerOnStopped();
      CleanLastPeerConnection();
      break;
    default:
      break;
  }
}

void P2PPeerConnectionChannel::OnIceGatheringChange(
    PeerConnectionInterface::IceGatheringState new_state) {
  LOG(LS_INFO) << "Ice gathering state changed: " << new_state;
}

void P2PPeerConnectionChannel::OnIceCandidate(
    const webrtc::IceCandidateInterface* candidate) {
  LOG(LS_INFO) << "On ice candidate";
  Json::Value signal;
  signal[kSessionDescriptionTypeKey] = "candidates";
  signal[kIceCandidateSdpMLineIndexKey] = candidate->sdp_mline_index();
  signal[kIceCandidateSdpMidKey] = candidate->sdp_mid();
  string sdp;
  if (!candidate->ToString(&sdp)) {
    LOG(LS_ERROR) << "Failed to serialize candidate";
    return;
  }
  signal[kIceCandidateSdpNameKey] = sdp;
  Json::Value json;
  json[kMessageTypeKey] = kChatSignal;
  json[kMessageDataKey] = signal;
  SendSignalingMessage(json, nullptr, nullptr);
}

void P2PPeerConnectionChannel::OnCreateSessionDescriptionSuccess(
    webrtc::SessionDescriptionInterface* desc) {
  LOG(LS_INFO) << "Create sdp success.";
  scoped_refptr<FunctionalSetSessionDescriptionObserver> observer =
      FunctionalSetSessionDescriptionObserver::Create(
          std::bind(
              &P2PPeerConnectionChannel::OnSetLocalSessionDescriptionSuccess,
              this),
          std::bind(
              &P2PPeerConnectionChannel::OnSetLocalSessionDescriptionFailure,
              this, std::placeholders::_1));
  SetSessionDescriptionMessage* msg =
      new SetSessionDescriptionMessage(observer.get(), desc);
  LOG(LS_INFO) << "Post set local desc";
  pc_thread_->Post(RTC_FROM_HERE, this, kMessageTypeSetLocalDescription, msg);
}

void P2PPeerConnectionChannel::OnCreateSessionDescriptionFailure(
    const std::string& error) {
  LOG(LS_INFO) << "Create sdp failed.";
  Stop(nullptr, nullptr);
}

void P2PPeerConnectionChannel::OnSetLocalSessionDescriptionSuccess() {
  LOG(LS_INFO) << "Set local sdp success.";
  {
    std::lock_guard<std::mutex> lock(is_creating_offer_mutex_);
    if (is_creating_offer_) {
      is_creating_offer_ = false;
    }
  }
  // Setting maximum bandwidth here
  ApplyBitrateSettings();
  auto desc = LocalDescription();
  string sdp;
  desc->ToString(&sdp);
  Json::Value signal;
  signal[kSessionDescriptionTypeKey] = desc->type();
  signal[kSessionDescriptionSdpKey] = sdp;
  Json::Value json;
  json[kMessageTypeKey] = kChatSignal;
  json[kMessageDataKey] = signal;
  SendSignalingMessage(json, nullptr, nullptr);
}

void P2PPeerConnectionChannel::OnSetLocalSessionDescriptionFailure(
    const std::string& error) {
  LOG(LS_INFO) << "Set local sdp failed.";
  Stop(nullptr, nullptr);
}

void P2PPeerConnectionChannel::OnSetRemoteSessionDescriptionSuccess() {
  PeerConnectionChannel::OnSetRemoteSessionDescriptionSuccess();
}

void P2PPeerConnectionChannel::OnSetRemoteSessionDescriptionFailure(
    const std::string& error) {
  LOG(LS_INFO) << "Set remote sdp failed.";
  Stop(nullptr, nullptr);
}

bool P2PPeerConnectionChannel::CheckNullPointer(
    uintptr_t pointer,
    std::function<void(std::unique_ptr<P2PException>)> on_failure) {
  if (pointer)
    return true;
  if (on_failure != nullptr) {
    event_queue_->PostTask([on_failure] {
      std::unique_ptr<P2PException> e(new P2PException(
          P2PException::kClientInvalidArgument, "Nullptr is not allowed."));
      on_failure(std::move(e));
    });
  }
  return false;
}

void P2PPeerConnectionChannel::TriggerOnStopped() {
  for (std::vector<P2PPeerConnectionChannelObserver*>::iterator it =
           observers_.begin();
       it != observers_.end(); it++) {
    (*it)->OnStopped(remote_id_);
  }
}

void P2PPeerConnectionChannel::CleanLastPeerConnection() {
  if (set_remote_sdp_task_) {
    delete set_remote_sdp_task_;
    set_remote_sdp_task_ = nullptr;
  }
  negotiation_needed_ = false;
  last_disconnect_ = std::chrono::time_point<std::chrono::system_clock>::max();
}

void P2PPeerConnectionChannel::Publish(
    std::shared_ptr<LocalStream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<P2PException>)> on_failure) {
  LOG(LS_INFO) << "Publish a local stream.";
  if (!CheckNullPointer((uintptr_t)stream.get(), on_failure)) {
    LOG(LS_INFO) << "Local stream cannot be nullptr.";
    return;
  }
  if (session_state_!=SessionState::kSessionStateConnected){
    std::string error_message(
        "Cannot publish a stream when connection is not established.");
    LOG(LS_WARNING) << error_message;
    if (on_failure) {
      event_queue_->PostTask([on_failure, error_message] {
        std::unique_ptr<P2PException> e(
            new P2PException(P2PException::kClientInvalidState, error_message));
        on_failure(std::move(e));
      });
    }
  }
  if (!remote_side_supports_plan_b_ &&
      published_streams_.size() + pending_publish_streams_.size() > 0) {
    if (on_failure != nullptr) {
      LOG(LS_WARNING) << "Remote side does not support Plan B, so at most one "
                         "audio/video track can be published.";
      event_queue_->PostTask([on_failure] {
        std::unique_ptr<P2PException> e(new P2PException(
            P2PException::kClientUnsupportedMethod,
            "Cannot publish multiple streams to remote side."));
        on_failure(std::move(e));
      });
    }
    return;
  }
  RTC_CHECK(stream->MediaStream());
  if (published_streams_.find(stream->MediaStream()->label()) !=
      published_streams_.end()) {
    if (on_failure) {
      event_queue_->PostTask([on_failure] {
        std::unique_ptr<P2PException> e(
            new P2PException(P2PException::kClientInvalidArgument,
                             "The stream is already published."));
        on_failure(std::move(e));
      });
    }
    return;
  }
  {
    std::lock_guard<std::mutex> lock(published_streams_mutex_);
    published_streams_.insert(stream->MediaStream()->label());
  }
  {
    std::lock_guard<std::mutex> lock(pending_publish_streams_mutex_);
    pending_publish_streams_.push_back(stream);
  }
  if (on_success) {
    event_queue_->PostTask([on_success] { on_success(); });
  }
  LOG(LS_INFO) << "Session state: " << session_state_;
  if (session_state_ == SessionState::kSessionStateConnected &&
      SignalingState() == PeerConnectionInterface::SignalingState::kStable)
    DrainPendingStreams();
}

void P2PPeerConnectionChannel::Unpublish(
    std::shared_ptr<LocalStream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<P2PException>)> on_failure) {
  if (!CheckNullPointer((uintptr_t)stream.get(), on_failure)) {
    LOG(LS_WARNING) << "Local stream cannot be nullptr.";
    return;
  }
  if (!remote_side_supports_remove_stream_) {
    if (on_failure != nullptr) {
      LOG(LS_WARNING) << "Remote side not support removeStream.";
      event_queue_->PostTask([on_failure] {
        std::unique_ptr<P2PException> e(
            new P2PException(P2PException::kClientUnsupportedMethod,
                             "Remote side does not support unpublish."));
        on_failure(std::move(e));
      });
    }
    return;
  }
  RTC_CHECK(stream->MediaStream());
  {
    std::lock_guard<std::mutex> lock(published_streams_mutex_);
    auto it = published_streams_.find(stream->MediaStream()->label());
    if (it == published_streams_.end()) {
      if (on_failure) {
        event_queue_->PostTask([on_failure] {
          std::unique_ptr<P2PException> e(
              new P2PException(P2PException::kClientInvalidArgument,
                               "The stream is not published."));
          on_failure(std::move(e));
        });
      }
      return;
    }
    published_streams_.erase(it);
  }
  {
    std::lock_guard<std::mutex> lock(pending_unpublish_streams_mutex_);
    pending_unpublish_streams_.push_back(stream);
  }
  if (on_success) {
    event_queue_->PostTask([on_success] { on_success(); });
  }
  if (session_state_ == SessionState::kSessionStateConnected &&
      SignalingState() == PeerConnectionInterface::SignalingState::kStable)
    DrainPendingStreams();
}

void P2PPeerConnectionChannel::Stop(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<P2PException>)> on_failure) {
  LOG(LS_INFO) << "Stop session.";
  switch (session_state_) {
    case kSessionStateConnecting:
    case kSessionStateConnected:
      pc_thread_->Post(RTC_FROM_HERE, this, kMessageTypeClosePeerConnection,
                       nullptr);
    case kSessionStateMatched:
      SendStop(nullptr, nullptr);
      ChangeSessionState(kSessionStateReady);
      break;
    case kSessionStateOffered:
      SendStop(nullptr, nullptr);
      ChangeSessionState(kSessionStateReady);
      TriggerOnStopped();
      break;
    default:
      if (on_failure != nullptr) {
        event_queue_->PostTask([on_failure] {
          std::unique_ptr<P2PException> e(
              new P2PException(P2PException::kClientInvalidState,
                               "Cannot stop a session haven't started."));
          on_failure(std::move(e));
        });
      }
      return;
  }
  if (on_success != nullptr) {
    event_queue_->PostTask([on_success] { on_success(); });
  }
}

void P2PPeerConnectionChannel::GetConnectionStats(
    std::function<void(std::shared_ptr<ConnectionStats>)> on_success,
    std::function<void(std::unique_ptr<P2PException>)> on_failure) {
  if (on_success == nullptr) {
    if (on_failure != nullptr) {
      event_queue_->PostTask([on_failure] {
        std::unique_ptr<P2PException> e(
            new P2PException(P2PException::kClientInvalidArgument,
                             "on_success cannot be nullptr. Please provide "
                             "on_success to get connection stats data."));
        on_failure(std::move(e));
      });
    }
    return;
  }
  if (session_state_ != kSessionStateConnected) {
    if (on_failure != nullptr) {
      event_queue_->PostTask([on_failure] {
        std::unique_ptr<P2PException> e(new P2PException(
            P2PException::kClientInvalidState,
            "Cannot get connection stats in this state. Please "
            "try it after connection is established."));
        on_failure(std::move(e));
      });
    }
    return;
  }
  LOG(LS_INFO) << "Get connection stats";
  rtc::scoped_refptr<FunctionalStatsObserver> observer =
      FunctionalStatsObserver::Create(std::move(on_success));
  GetStatsMessage* stats_message = new GetStatsMessage(
      observer, nullptr,
      webrtc::PeerConnectionInterface::kStatsOutputLevelStandard);
  pc_thread_->Post(RTC_FROM_HERE, this, kMessageTypeGetStats, stats_message);
}

void P2PPeerConnectionChannel::DrainPendingStreams() {
  LOG(LS_INFO) << "Draining pending stream";
  {
    std::lock_guard<std::mutex> lock(pending_publish_streams_mutex_);
    for (auto it = pending_publish_streams_.begin();
         it != pending_publish_streams_.end(); ++it) {
      std::shared_ptr<LocalStream> stream = *it;
      scoped_refptr<webrtc::MediaStreamInterface> media_stream =
          stream->MediaStream();
      Json::Value json;
      json[kMessageTypeKey] = kStreamType;
      Json::Value stream_info;
      stream_info[kStreamIdKey] = media_stream->label();
      if (stream->GetStreamDeviceType() ==
          ics::base::LocalStream::StreamDeviceType::
              kStreamDeviceTypeScreen) {
        stream_info[kStreamTypeKey] = "screen";
      } else {
        stream_info[kStreamTypeKey] = "video";
      }
      json[kMessageDataKey] = stream_info;
      SendSignalingMessage(json, nullptr, nullptr);
      rtc::ScopedRefMessageData<MediaStreamInterface>* param =
          new rtc::ScopedRefMessageData<MediaStreamInterface>(media_stream);
      LOG(LS_INFO) << "Post add stream";
      pc_thread_->Post(RTC_FROM_HERE, this, kMessageTypeAddStream, param);
    }
    pending_publish_streams_.clear();
  }
  {
    std::lock_guard<std::mutex> lock(pending_unpublish_streams_mutex_);
    LOG(LS_INFO) << "Get unpublish stream lock.";
    for (auto it = pending_unpublish_streams_.begin();
         it != pending_unpublish_streams_.end(); ++it) {
      LOG(LS_INFO) << "Remove a stream from peer connection.";
      std::shared_ptr<LocalStream> stream = *it;
      scoped_refptr<webrtc::MediaStreamInterface> media_stream =
          stream->MediaStream();
      rtc::ScopedRefMessageData<MediaStreamInterface>* param =
          new rtc::ScopedRefMessageData<MediaStreamInterface>(media_stream);
      LOG(LS_INFO) << "Post remove stream";
      pc_thread_->Post(RTC_FROM_HERE, this, kMessageTypeRemoveStream, param);
    }
    pending_unpublish_streams_.clear();
  }
}

void P2PPeerConnectionChannel::SendAcceptance(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<P2PException>)> on_failure) {
  Json::Value json;
  json[kMessageTypeKey] = kChatAccept;
  Json::Value ua = UaInfo();
  json[kUaKey] = ua;
  SendSignalingMessage(json, on_success, on_failure);
}

void P2PPeerConnectionChannel::SendStop(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<P2PException>)> on_failure) {
  LOG(LS_INFO) << "Send stop.";
  Json::Value json;
  json[kMessageTypeKey] = kChatStop;
  SendSignalingMessage(json, on_success, on_failure);
}

void P2PPeerConnectionChannel::SendDeny(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<P2PException>)> on_failure) {
  Json::Value json;
  json[kMessageTypeKey] = kChatDeny;
  SendSignalingMessage(json, on_success, on_failure);
}

void P2PPeerConnectionChannel::ClosePeerConnection() {
  LOG(LS_INFO) << "Close peer connection.";
  RTC_CHECK(pc_thread_);
  pc_thread_->Send(RTC_FROM_HERE, this, kMessageTypeClosePeerConnection, nullptr);
  ChangeSessionState(kSessionStateReady);
}

void P2PPeerConnectionChannel::CheckWaitedList() {
  LOG(LS_INFO) << "CheckWaitedList";
  if (!pending_publish_streams_.empty() ||
      !pending_unpublish_streams_.empty()) {
    DrainPendingStreams();
  } else if (negotiation_needed_) {
    RTC_DCHECK(is_caller_);
    CreateOffer();
  }
}

void P2PPeerConnectionChannel::OnDataChannelStateChange() {
  RTC_CHECK(data_channel_);
  if (data_channel_->state() ==
      webrtc::DataChannelInterface::DataState::kOpen) {
    DrainPendingMessages();
  }
}

void P2PPeerConnectionChannel::OnDataChannelMessage(
    const webrtc::DataBuffer& buffer) {
  if (buffer.binary) {
    LOG(LS_WARNING) << "Binary data is not supported.";
    return;
  }
  std::string message =
      std::string(buffer.data.data<char>(), buffer.data.size());
  for (std::vector<P2PPeerConnectionChannelObserver*>::iterator it =
           observers_.begin();
       it != observers_.end(); ++it) {
    (*it)->OnData(remote_id_, message);
  }
}

void P2PPeerConnectionChannel::CreateDataChannel(const std::string& label) {
  rtc::TypedMessageData<std::string>* data =
      new rtc::TypedMessageData<std::string>(label);
  pc_thread_->Post(RTC_FROM_HERE, this, kMessageTypeCreateDataChannel, data);
}

void P2PPeerConnectionChannel::Send(
    const std::string& message,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<P2PException>)> on_failure) {
  if (data_channel_ != nullptr &&
      data_channel_->state() ==
          webrtc::DataChannelInterface::DataState::kOpen) {
    data_channel_->Send(CreateDataBuffer(message));
    LOG(LS_INFO) << "Send message " << message;
  } else {
    {
      std::lock_guard<std::mutex> lock(pending_messages_mutex_);
      std::shared_ptr<std::string> message_copy(
          std::make_shared<std::string>(message));
      pending_messages_.push_back(message_copy);
    }
    if (data_channel_ == nullptr) {  // Otherwise, wait for data channel ready.
      CreateDataChannel(kDataChannelLabelForTextMessage);
    }
  }
  if (on_success) {
    event_queue_->PostTask([on_success] { on_success(); });
  }
}

webrtc::DataBuffer P2PPeerConnectionChannel::CreateDataBuffer(
    const std::string& data) {
  rtc::CopyOnWriteBuffer buffer(data.c_str(), data.length());
  webrtc::DataBuffer data_buffer(buffer, false);
  return data_buffer;
}

void P2PPeerConnectionChannel::DrainPendingMessages() {
  LOG(LS_INFO) << "Draining pending messages. Message queue size: "
               << pending_messages_.size();
  RTC_CHECK(data_channel_);
  {
    std::lock_guard<std::mutex> lock(pending_messages_mutex_);
    for (auto it = pending_messages_.begin(); it != pending_messages_.end();
         ++it) {
      data_channel_->Send(CreateDataBuffer(**it));
    }
    pending_messages_.clear();
  }
}

void P2PPeerConnectionChannel::HandleRemoteCapability(Json::Value& ua) {
  Json::Value runtime;
  rtc::GetValueFromJsonObject(ua, kUaRuntimeKey, &runtime);
  std::string runtime_name;
  rtc::GetStringFromJsonObject(runtime, kUaRuntimeNameKey, &runtime_name);
  if (runtime_name.compare("FireFox") == 0) {
    remote_side_supports_remove_stream_ = false;
    remote_side_supports_plan_b_ = false;
  } else {
    remote_side_supports_remove_stream_ = true;
    remote_side_supports_plan_b_ = true;
  }
  LOG(LS_INFO) << "Remote side supports removing stream? "
               << remote_side_supports_remove_stream_;
  LOG(LS_INFO) << "Remote side supports WebRTC Plan B? "
               << remote_side_supports_plan_b_;
}
}
}
