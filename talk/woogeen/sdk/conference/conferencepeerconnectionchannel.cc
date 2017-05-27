/*
 * Intel License
 */

#include <vector>
#include <thread>
#include <future>

#include "webrtc/base/logging.h"
#include "webrtc/media/base/videocommon.h"
#include "webrtc/media/base/videocapturer.h"

#include "talk/woogeen/sdk/base/functionalobserver.h"
#include "talk/woogeen/sdk/base/mediautils.h"
#include "talk/woogeen/sdk/base/peerconnectiondependencyfactory.h"
#include "talk/woogeen/sdk/conference/conferencepeerconnectionchannel.h"
#include "talk/woogeen/sdk/include/cpp/woogeen/conference/remotemixedstream.h"

namespace woogeen {
namespace conference {

using std::string;

enum ConferencePeerConnectionChannel::SessionState : int {
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

enum ConferencePeerConnectionChannel::NegotiationState : int {
  kNegotiationStateNone = 1,  // Indicates not in renegotiation.
  kNegotiationStateSent,  // Indicates a negotiation request has been sent to
                          // remote user.
  kNegotiationStateReceived,  // Indicates local side has received a negotiation
                              // request from remote user.
  kNegotiationStateAccepted,  // Indicates local side has accepted remote user's
                              // negotiation request.
};

// Const value for messages
const int kSessionIdBase =
    104;  // Not sure why it should be 104, just according to JavaScript SDK.
const int kMessageSeqBase = 1;
const int kTiebreakerUpperBound = 429496723;  // ditto

// Stream option member key
const string kStreamOptionStreamIdKey = "streamId";
const string kStreamOptionStateKey = "state";
const string kStreamOptionDataKey = "type";
const string kStreamOptionAudioKey = "audio";
const string kStreamOptionVideoKey = "video";
const string kStreamOptionScreenKey = "screen";
const string kStreamOptionAttributesKey = "attributes";

// Session description member key
const string kSessionDescriptionMessageTypeKey = "messageType";
const string kSessionDescriptionSdpKey = "sdp";
const string kSessionDescriptionOfferSessionIdKey = "offererSessionId";
const string kSessionDescriptionAnswerSessionIdKey = "answerSessionId";
const string kSessionDescriptionSeqKey = "seq";
const string kSessionDescriptionTiebreakerKey = "tiebreaker";

// ICE candidate member key
const string kIceCandidateSdpMidKey = "sdpMid";
const string kIceCandidateSdpMLineIndexKey = "sdpMLineIndex";
const string kIceCandidateSdpNameKey = "candidate";

ConferencePeerConnectionChannel::ConferencePeerConnectionChannel(
    PeerConnectionChannelConfiguration& configuration,
    std::shared_ptr<ConferenceSocketSignalingChannel> signaling_channel)
    : PeerConnectionChannel(configuration),
      signaling_channel_(signaling_channel),
      session_id_(kSessionIdBase),
      message_seq_(kMessageSeqBase),
      ice_restart_needed_(false),
      connected_(false) {
  InitializePeerConnection();
  RTC_CHECK(signaling_channel_);
}

ConferencePeerConnectionChannel::~ConferencePeerConnectionChannel() {
  LOG(LS_INFO) << "Deconstruct conference peer connection channel";
  if (published_stream_)
    Unpublish(published_stream_, nullptr, nullptr);
  if (subscribed_stream_)
    Unsubscribe(subscribed_stream_, nullptr, nullptr);
}

void ConferencePeerConnectionChannel::AddObserver(
    ConferencePeerConnectionChannelObserver& observer) {
  const std::lock_guard<std::mutex> lock(observers_mutex_);
  std::vector<std::reference_wrapper<ConferencePeerConnectionChannelObserver>>::
      iterator it = std::find_if(
          observers_.begin(), observers_.end(),
          [&](std::reference_wrapper<ConferencePeerConnectionChannelObserver> o)
              -> bool { return &observer == &(o.get()); });
  if (it != observers_.end()) {
    LOG(LS_WARNING) << "Adding duplicate observer.";
    return;
  }
  observers_.push_back(observer);
}

void ConferencePeerConnectionChannel::RemoveObserver(
    ConferencePeerConnectionChannelObserver& observer) {
  const std::lock_guard<std::mutex> lock(observers_mutex_);
  observers_.erase(std::find_if(
      observers_.begin(), observers_.end(),
      [&](std::reference_wrapper<ConferencePeerConnectionChannelObserver> o)
          -> bool { return &observer == &(o.get()); }));
}

void ConferencePeerConnectionChannel::CreateOffer() {
  LOG(LS_INFO) << "Create offer.";
  scoped_refptr<FunctionalCreateSessionDescriptionObserver> observer =
      FunctionalCreateSessionDescriptionObserver::Create(
          std::bind(&ConferencePeerConnectionChannel::
                        OnCreateSessionDescriptionSuccess,
                    this, std::placeholders::_1),
          std::bind(&ConferencePeerConnectionChannel::
                        OnCreateSessionDescriptionFailure,
                    this, std::placeholders::_1));
  rtc::TypedMessageData<
      scoped_refptr<FunctionalCreateSessionDescriptionObserver>>* data =
      new rtc::TypedMessageData<
          scoped_refptr<FunctionalCreateSessionDescriptionObserver>>(observer);
  LOG(LS_INFO) << "Post create offer";
  pc_thread_->Post(RTC_FROM_HERE, this, kMessageTypeCreateOffer, data);
}

void ConferencePeerConnectionChannel::IceRestart() {
  if (SignalingState() == PeerConnectionInterface::SignalingState::kStable) {
    DoIceRestart();
  } else {
    ice_restart_needed_ = true;
  }
}

void ConferencePeerConnectionChannel::DoIceRestart() {
  LOG(LS_INFO) << "ICE restart";
  RTC_DCHECK(SignalingState() ==
             PeerConnectionInterface::SignalingState::kStable);
  media_constraints_.SetMandatory(
      webrtc::MediaConstraintsInterface::kIceRestart, true);
  this->CreateOffer();
}

void ConferencePeerConnectionChannel::CreateAnswer() {
  LOG(LS_INFO) << "Create answer.";
  scoped_refptr<FunctionalCreateSessionDescriptionObserver> observer =
      FunctionalCreateSessionDescriptionObserver::Create(
          std::bind(&ConferencePeerConnectionChannel::
                        OnCreateSessionDescriptionSuccess,
                    this, std::placeholders::_1),
          std::bind(&ConferencePeerConnectionChannel::
                        OnCreateSessionDescriptionFailure,
                    this, std::placeholders::_1));
  rtc::TypedMessageData<
      scoped_refptr<FunctionalCreateSessionDescriptionObserver>>*
      message_observer = new rtc::TypedMessageData<
          scoped_refptr<FunctionalCreateSessionDescriptionObserver>>(observer);
  LOG(LS_INFO) << "Post create answer";
  pc_thread_->Post(RTC_FROM_HERE, this, kMessageTypeCreateAnswer, message_observer);
}

void ConferencePeerConnectionChannel::OnSignalingChange(
    PeerConnectionInterface::SignalingState new_state) {
  LOG(LS_INFO) << "Signaling state changed: " << new_state;
  signaling_state_ = new_state;
  if (new_state == webrtc::PeerConnectionInterface::SignalingState::kStable) {
    if (ice_restart_needed_) {
      ice_restart_needed_ = false;
      {
        std::lock_guard<std::mutex> lock(candidates_mutex_);
        ice_candidates_.clear();
      }
      DoIceRestart();
    } else {
      DrainIceCandidates();
    }
  }
}

void ConferencePeerConnectionChannel::OnAddStream(
    MediaStreamInterface* stream) {
  LOG(LS_INFO) << "On add stream.";
  if (subscribed_stream_ != nullptr)
    subscribed_stream_->MediaStream(stream);
}

void ConferencePeerConnectionChannel::OnRemoveStream(
    MediaStreamInterface* stream) {}

void ConferencePeerConnectionChannel::OnDataChannel(
    webrtc::DataChannelInterface* data_channel) {}

void ConferencePeerConnectionChannel::OnRenegotiationNeeded() {}

void ConferencePeerConnectionChannel::OnIceConnectionChange(
    PeerConnectionInterface::IceConnectionState new_state) {
  LOG(LS_INFO) << "Ice connection state changed: " << new_state;
  if (new_state == PeerConnectionInterface::kIceConnectionConnected ||
      new_state == PeerConnectionInterface::kIceConnectionCompleted) {
    if (publish_success_callback_) {
      std::thread t(publish_success_callback_);
      t.detach();
    } else if (subscribe_success_callback_) {
      std::thread t(subscribe_success_callback_, subscribed_stream_);
      t.detach();
    }
    connected_ = true;
  } else if (new_state == PeerConnectionInterface::kIceConnectionClosed) {
    if (!connected_ && failure_callback_) {
      std::unique_ptr<ConferenceException> e(new ConferenceException(
          ConferenceException::kUnknown, "ICE failed."));
      std::thread t(failure_callback_, std::move(e));
      t.detach();
    } else if (connected_) {
      OnStreamError(std::string("ICE connection state changed to closed."));
    }
    connected_ = false;
  } else {
    return;
  }
  publish_success_callback_ = nullptr;
  subscribe_success_callback_ = nullptr;
  failure_callback_ = nullptr;
}

void ConferencePeerConnectionChannel::OnIceGatheringChange(
    PeerConnectionInterface::IceGatheringState new_state) {
  LOG(LS_INFO) << "Ice gathering state changed: " << new_state;
}

void ConferencePeerConnectionChannel::OnIceCandidate(
    const webrtc::IceCandidateInterface* candidate) {
  LOG(LS_INFO) << "On ice candidate";
  string candidate_string;
  candidate->ToString(&candidate_string);
  candidate_string.insert(0, "a=");
  sio::message::ptr message = sio::object_message::create();
  message->get_map()["streamId"] = sio::string_message::create(GetStreamId());
  sio::message::ptr sdp_message = sio::object_message::create();
  sdp_message->get_map()["type"] = sio::string_message::create("candidate");
  sio::message::ptr candidate_message = sio::object_message::create();
  candidate_message->get_map()["sdpMLineIndex"] =
      sio::int_message::create(candidate->sdp_mline_index());
  candidate_message->get_map()["sdpMid"] =
      sio::string_message::create(candidate->sdp_mid());
  candidate_message->get_map()["candidate"] =
      sio::string_message::create(candidate_string);
  sdp_message->get_map()["candidate"] = candidate_message;
  message->get_map()["msg"] = sdp_message;
  if (signaling_state_ ==
      webrtc::PeerConnectionInterface::SignalingState::kStable) {
    signaling_channel_->SendSdp(message, nullptr, nullptr);
  } else {
    ice_candidates_.push_back(message);
  }
}

void ConferencePeerConnectionChannel::OnCreateSessionDescriptionSuccess(
    webrtc::SessionDescriptionInterface* desc) {
  LOG(LS_INFO) << "Create sdp success.";
  scoped_refptr<FunctionalSetSessionDescriptionObserver> observer =
      FunctionalSetSessionDescriptionObserver::Create(
          std::bind(&ConferencePeerConnectionChannel::
                        OnSetLocalSessionDescriptionSuccess,
                    this),
          std::bind(&ConferencePeerConnectionChannel::
                        OnSetLocalSessionDescriptionFailure,
                    this, std::placeholders::_1));
  SetSessionDescriptionMessage* msg =
      new SetSessionDescriptionMessage(observer.get(), desc);
  LOG(LS_INFO) << "Post set local desc";
  pc_thread_->Post(RTC_FROM_HERE, this, kMessageTypeSetLocalDescription, msg);
}

void ConferencePeerConnectionChannel::OnCreateSessionDescriptionFailure(
    const std::string& error) {
  LOG(LS_INFO) << "Create sdp failed.";
}

void ConferencePeerConnectionChannel::OnSetLocalSessionDescriptionSuccess() {
  LOG(LS_INFO) << "Set local sdp success.";
  // For conference, it's now OK to set bandwidth
  ApplyBitrateSettings();
  auto desc = LocalDescription();
  string sdp;
  desc->ToString(&sdp);
  sio::message::ptr message = sio::object_message::create();
  LOG(LS_INFO) << "Local SDP for stream: " << GetStreamId();
  message->get_map()["streamId"] = sio::string_message::create(GetStreamId());
  sio::message::ptr sdp_message = sio::object_message::create();
  sdp_message->get_map()["type"] = sio::string_message::create(desc->type());
  sdp_message->get_map()["sdp"] = sio::string_message::create(sdp);
  message->get_map()["msg"] = sdp_message;
  signaling_channel_->SendSdp(message, nullptr, nullptr);
}

void ConferencePeerConnectionChannel::OnSetLocalSessionDescriptionFailure(
    const std::string& error) {
  LOG(LS_INFO) << "Set local sdp failed.";
}

void ConferencePeerConnectionChannel::OnSetRemoteSessionDescriptionSuccess() {
  PeerConnectionChannel::OnSetRemoteSessionDescriptionSuccess();
}

void ConferencePeerConnectionChannel::OnSetRemoteSessionDescriptionFailure(
    const std::string& error) {
  LOG(LS_INFO) << "Set remote sdp failed.";
}

void ConferencePeerConnectionChannel::SetRemoteDescription(
    const std::string& type,
    const std::string& sdp) {
  webrtc::SessionDescriptionInterface* desc(webrtc::CreateSessionDescription(
      "answer", sdp,
      nullptr));  // TODO(jianjun): change answer to type.toLowerCase.
  if (!desc) {
    LOG(LS_ERROR) << "Failed to create session description.";
    return;
  }
  scoped_refptr<FunctionalSetSessionDescriptionObserver> observer =
      FunctionalSetSessionDescriptionObserver::Create(
          std::bind(&ConferencePeerConnectionChannel::OnSetRemoteSessionDescriptionSuccess,
                    this),
          std::bind(&ConferencePeerConnectionChannel::OnSetRemoteSessionDescriptionFailure,
                    this, std::placeholders::_1));
  SetSessionDescriptionMessage* msg =
      new SetSessionDescriptionMessage(observer.get(), desc);
  LOG(LS_INFO) << "Post set remote desc";
  pc_thread_->Post(RTC_FROM_HERE, this, kMessageTypeSetRemoteDescription, msg);
}

bool ConferencePeerConnectionChannel::CheckNullPointer(
    uintptr_t pointer,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (pointer)
    return true;
  if (on_failure != nullptr) {
    std::unique_ptr<ConferenceException> e(new ConferenceException(
        ConferenceException::kUnknown, "Nullptr is not allowed."));
    on_failure(std::move(e));
  }
  return false;
}

void ConferencePeerConnectionChannel::Publish(
    std::shared_ptr<LocalStream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  LOG(LS_INFO) << "Publish a local stream.";
  RTC_DCHECK(!subscribe_success_callback_);
  published_stream_ = stream;
  if ((!CheckNullPointer((uintptr_t)stream.get(), on_failure)) ||
      (!CheckNullPointer((uintptr_t)stream->MediaStream(), on_failure))) {
    LOG(LS_INFO) << "Local stream cannot be nullptr.";
    return;
  }

  if (publish_success_callback_) {
    if (on_failure) {
      std::unique_ptr<ConferenceException> e(new ConferenceException(
          ConferenceException::kUnknown, "Publishing this stream."));
      on_failure(std::move(e));
    }
  }
  publish_success_callback_ = on_success;
  failure_callback_ = on_failure;

  media_constraints_.SetMandatory(
      webrtc::MediaConstraintsInterface::kOfferToReceiveAudio, false);
  media_constraints_.SetMandatory(
      webrtc::MediaConstraintsInterface::kOfferToReceiveVideo, false);

  sio::message::ptr options = sio::object_message::create();
  options->get_map()["state"] = sio::string_message::create("erizo");
  sio::message::ptr attributes_ptr = sio::object_message::create();
  for (auto const& attr : stream->Attributes()) {
    attributes_ptr->get_map()[attr.first] =
        sio::string_message::create(attr.second);
  }
  options->get_map()[kStreamOptionAttributesKey] = attributes_ptr;
  if (stream->MediaStream()->GetAudioTracks().size() == 0) {
    options->get_map()["audio"] = sio::bool_message::create(false);
  } else {
    options->get_map()["audio"] = sio::bool_message::create(true);
  }
  if (stream->MediaStream()->GetVideoTracks().size() == 0) {
    options->get_map()["video"] = sio::bool_message::create(false);
    SendPublishMessage(options, stream, on_failure);
  } else {
    sio::message::ptr video_options = sio::object_message::create();
    if (stream->GetStreamDeviceType() ==
        woogeen::base::LocalStream::StreamDeviceType::kStreamDeviceTypeScreen) {
      video_options->get_map()["device"] =
          sio::string_message::create("screen");
    } else {
      video_options->get_map()["device"] =
          sio::string_message::create("camera");
    }
    options->get_map()["video"] = video_options;
    SendPublishMessage(options, stream, on_failure);
  }
}

void ConferencePeerConnectionChannel::Subscribe(
    std::shared_ptr<RemoteStream> stream,
    const SubscribeOptions& subscribe_options,
    std::function<void(std::shared_ptr<RemoteStream> stream)> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  LOG(LS_INFO) << "Subscribe a remote stream. It has audio? "
               << stream->has_audio_ << ", has video? " << stream->has_video_;
  RTC_DCHECK(!publish_success_callback_);
  subscribed_stream_ = stream;
  if (!CheckNullPointer((uintptr_t)stream.get(), on_failure)) {
    LOG(LS_ERROR) << "Local stream cannot be nullptr.";
    return;
  }
  if (subscribe_success_callback_) {
    if (on_failure) {
      std::unique_ptr<ConferenceException> e(new ConferenceException(
          ConferenceException::kUnknown, "Subscribing this stream."));
      on_failure(std::move(e));
    }
  }
  subscribe_success_callback_ = on_success;
  failure_callback_ = on_failure;
  sio::message::ptr sio_options = sio::object_message::create();
  sio_options->get_map()["streamId"] =
      sio::string_message::create(stream->Id());
  sio::message::ptr video_options = sio::object_message::create();
  if (stream->has_video_) {
    if (subscribe_options.resolution.width != 0 &&
        subscribe_options.resolution.height != 0) {
      sio::message::ptr resolution_options = sio::object_message::create();
      resolution_options->get_map()["width"] =
          sio::int_message::create(subscribe_options.resolution.width);
      resolution_options->get_map()["height"] =
          sio::int_message::create(subscribe_options.resolution.height);
      video_options->get_map()["resolution"] = resolution_options;
    }
    std::string quality_level("Standard");
    switch (subscribe_options.video_quality_level) {
      case SubscribeOptions::VideoQualityLevel::kStandard:
        break;
      case SubscribeOptions::VideoQualityLevel::kBestQuality:
        quality_level = "BestQuality";
        break;
      case SubscribeOptions::VideoQualityLevel::kBetterQuality:
        quality_level = "BetterQuality";
        break;
      case SubscribeOptions::VideoQualityLevel::kBetterSpeed:
        quality_level = "BetterSpeed";
        break;
      case SubscribeOptions::VideoQualityLevel::kBestSpeed:
        quality_level = "BestSpeed";
        break;
      default:
        RTC_NOTREACHED();
        break;
    }
    sio::message::ptr quality_options =
        sio::string_message::create(quality_level);
    video_options->get_map()["quality_level"] = quality_options;
    sio_options->get_map()["video"] = video_options;
  } else {
    LOG(LS_INFO) << "Subscribe an audio only stream.";
    sio_options->get_map()["video"] = sio::bool_message::create(false);
  }
  sio_options->get_map()["audio"] =
      sio::bool_message::create(stream->has_audio_);
  media_constraints_.SetMandatory(
      webrtc::MediaConstraintsInterface::kOfferToReceiveAudio,
      stream->has_audio_);
  media_constraints_.SetMandatory(
      webrtc::MediaConstraintsInterface::kOfferToReceiveVideo,
      stream->has_video_);
  signaling_channel_->SendInitializationMessage(sio_options, "", [this]() {
    CreateOffer();
  }, on_failure);  // TODO: on_failure
}

void ConferencePeerConnectionChannel::Subscribe(
    std::shared_ptr<RemoteMixedStream> stream,
    const SubscribeOptions& subscribe_options,
    std::function<void(std::shared_ptr<RemoteStream> stream)> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (subscribe_options.resolution.width != 0 ||
      subscribe_options.resolution.height != 0) {
    auto supported_formats = stream->SupportedVideoFormats();
    if (std::find_if(supported_formats.begin(), supported_formats.end(),
                     [&](const VideoFormat& format) {
                       return format.resolution == subscribe_options.resolution;
                     }) != supported_formats.end()) {
      Subscribe(std::static_pointer_cast<RemoteStream>(stream),
                subscribe_options, on_success, on_failure);
    } else {
      LOG(LS_ERROR) << "Subscribe unsupported resolution.";
      if (on_failure != nullptr) {
        std::unique_ptr<ConferenceException> e(new ConferenceException(
            ConferenceException::kUnknown, "Unsupported resolution."));
        on_failure(std::move(e));
      }
    }
  } else {
    Subscribe(std::static_pointer_cast<RemoteStream>(stream), subscribe_options,
              on_success, on_failure);
  }
}

void ConferencePeerConnectionChannel::Subscribe(
    std::shared_ptr<RemoteScreenStream> stream,
    const SubscribeOptions& subscribe_options,
    std::function<void(std::shared_ptr<RemoteStream> stream)> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (subscribe_options.resolution.width != 0 ||
      subscribe_options.resolution.height != 0) {
    LOG(LS_ERROR) << "Screen sharing stream does not support resolution settings. "
                     "Please don't change resolution.";
    if (on_failure != nullptr) {
      std::unique_ptr<ConferenceException> e(new ConferenceException(
          ConferenceException::kUnknown,
          "Cannot specify resolution settings for screen sharing stream."));
      on_failure(std::move(e));
      }
  } else {
    Subscribe(std::static_pointer_cast<RemoteStream>(stream),
                  subscribe_options, on_success, on_failure);
  }
}


void ConferencePeerConnectionChannel::Subscribe(
    std::shared_ptr<RemoteCameraStream> stream,
    const SubscribeOptions& subscribe_options,
    std::function<void(std::shared_ptr<RemoteStream> stream)> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (subscribe_options.resolution.width != 0 ||
      subscribe_options.resolution.height != 0) {
    LOG(LS_ERROR) << "Camera stream does not support resolution settings. "
                     "Please don't change resolution.";
    if (on_failure != nullptr) {
      std::unique_ptr<ConferenceException> e(new ConferenceException(
          ConferenceException::kUnknown,
          "Cannot specify resolution settings for camera stream."));
      on_failure(std::move(e));
      }
  } else {
    Subscribe(std::static_pointer_cast<RemoteStream>(stream),
                  subscribe_options, on_success, on_failure);
  }
}

void ConferencePeerConnectionChannel::Unpublish(
    std::shared_ptr<LocalStream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (!CheckNullPointer((uintptr_t)stream.get(), on_failure)) {
    LOG(LS_ERROR) << "Local stream cannot be nullptr.";
    return;
  }
  if (published_stream_ == nullptr || stream->Id() != published_stream_->Id()) {
    LOG(LS_ERROR) << "Stream ID doesn't match published stream.";
    if (on_failure != nullptr) {
      std::unique_ptr<ConferenceException> e(new ConferenceException(
          ConferenceException::kUnknown, "Invalid stream to be unpublished."));
      on_failure(std::move(e));
    }
    return;
  }
  if (publish_success_callback_ != nullptr) {  // Publishing
    if (on_failure != nullptr) {
      std::unique_ptr<ConferenceException> e(new ConferenceException(
          ConferenceException::kUnknown,
          "Cannot unpublish a stream during publishing."));
      on_failure(std::move(e));
    }
    return;
  }
  connected_ = false;
  signaling_channel_->SendStreamEvent("unpublish", stream->Id(), on_success,
                                      on_failure);
}

void ConferencePeerConnectionChannel::Unsubscribe(
    std::shared_ptr<RemoteStream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (!CheckNullPointer((uintptr_t)stream.get(), on_failure)) {
    LOG(LS_ERROR) << "Remote stream cannot be nullptr.";
    return;
  }
  if (subscribed_stream_ == nullptr ||
      stream->Id() != subscribed_stream_->Id()) {
    LOG(LS_ERROR) << "Stream ID doesn't match subscribed stream.";
    if (on_failure != nullptr) {
      std::unique_ptr<ConferenceException> e(new ConferenceException(
          ConferenceException::kUnknown, "Invalid stream to be unsubscribed."));
      on_failure(std::move(e));
    }
    return;
  }
  if (subscribe_success_callback_ != nullptr) {  // Subscribing
    if (on_failure != nullptr) {
      std::unique_ptr<ConferenceException> e(new ConferenceException(
          ConferenceException::kUnknown,
          "Cannot unsubscribe a stream during subscribing."));
      on_failure(std::move(e));
    }
    return;
  }
  connected_ = false;
  signaling_channel_->SendStreamEvent("unsubscribe", stream->Id(), on_success,
                                      on_failure);
}

void ConferencePeerConnectionChannel::SendStreamControlMessage(
    const std::shared_ptr<Stream> stream,
    const std::string& in_action,
    const std::string& out_action,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure)
    const {
  std::string action = "";
  if (stream == published_stream_)
    action = out_action;
  else if (stream == subscribed_stream_)
    action = in_action;
  else
    ASSERT(false);
  signaling_channel_->SendStreamControlMessage(stream->Id(), action, on_success,
                                               on_failure);
}

void ConferencePeerConnectionChannel::PlayAudio(
    std::shared_ptr<Stream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  SendStreamControlMessage(stream, "audio-in-on", "audio-out-on", on_success,
                           on_failure);
}

void ConferencePeerConnectionChannel::PauseAudio(
    std::shared_ptr<Stream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  SendStreamControlMessage(stream, "audio-in-off", "audio-out-off", on_success,
                           on_failure);
}

void ConferencePeerConnectionChannel::PlayVideo(
    std::shared_ptr<Stream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  SendStreamControlMessage(stream, "video-in-on", "video-out-on", on_success,
                           on_failure);
}

void ConferencePeerConnectionChannel::PauseVideo(
    std::shared_ptr<Stream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  SendStreamControlMessage(stream, "video-in-off", "video-out-off", on_success,
                           on_failure);
}

void ConferencePeerConnectionChannel::Stop(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  LOG(LS_INFO) << "Stop session.";
}

void ConferencePeerConnectionChannel::GetConnectionStats(
    std::shared_ptr<Stream> stream,
    std::function<void(std::shared_ptr<ConnectionStats>)> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  scoped_refptr<FunctionalStatsObserver> observer = FunctionalStatsObserver::Create(on_success);
  GetStatsMessage* stats_message = new GetStatsMessage(
      observer.get(), stream->MediaStream(),
      webrtc::PeerConnectionInterface::kStatsOutputLevelStandard);
  pc_thread_->Post(RTC_FROM_HERE, this, kMessageTypeGetStats, stats_message);
}

void ConferencePeerConnectionChannel::OnSignalingMessage(
    sio::message::ptr message) {
  if (message == nullptr || message->get_flag() != sio::message::flag_object) {
    LOG(LS_ERROR) << "Received unknown message from MCU.";
    return;
  }
  if (message->get_map().find("type") == message->get_map().end()) {
      LOG(LS_INFO) << "Ignore erizo message without type from MCU";
      return;
  }
  const std::string type = message->get_map()["type"]->get_string();
  LOG(LS_INFO) << "On signaling message: " << type;
  if (type == "answer") {
    const std::string sdp = message->get_map()["sdp"]->get_string();
    SetRemoteDescription(type, sdp);
  } else if (type == "failed") {
    LOG(LS_ERROR) << "Publish or subscribe stream failed.";
  } else if (type == "ready") {
    LOG(LS_INFO) << "Received ready.";
  }
}

void ConferencePeerConnectionChannel::DrainIceCandidates() {
  std::lock_guard<std::mutex> lock(candidates_mutex_);
  for (auto it = ice_candidates_.begin(); it != ice_candidates_.end(); ++it) {
    signaling_channel_->SendSdp(*it, nullptr, nullptr);
  }
  ice_candidates_.clear();
}

void ConferencePeerConnectionChannel::SetStreamId(const std::string& id) {
  LOG(LS_INFO) << "Setting stream ID " << id;
  if (published_stream_ == nullptr) {
    ASSERT(false);
  } else {
    published_stream_->Id(id);
  }
}

std::string ConferencePeerConnectionChannel::GetStreamId() const {
  if (subscribed_stream_ != nullptr) {
    return subscribed_stream_->Id();
  } else {
    RTC_CHECK(published_stream_ != nullptr);
    return published_stream_->Id();
  }
}

int ConferencePeerConnectionChannel::RandomInt(int lower_bound,
                                               int upper_bound) {
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<int> dist(1, kTiebreakerUpperBound);
  return dist(mt);
}

void ConferencePeerConnectionChannel::SendPublishMessage(
    sio::message::ptr options,
    std::shared_ptr<LocalStream> stream,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  signaling_channel_->SendInitializationMessage(
      options, stream->MediaStream()->label(),
      [stream, this]() {
        rtc::ScopedRefMessageData<MediaStreamInterface>* param =
            new rtc::ScopedRefMessageData<MediaStreamInterface>(
                stream->MediaStream());
        pc_thread_->Post(RTC_FROM_HERE, this, kMessageTypeAddStream, param);
        CreateOffer();
      },
      on_failure);
}

void ConferencePeerConnectionChannel::OnNetworksChanged(){
  LOG(LS_INFO) << "ConferencePeerConnectionChannel::OnNetworksChanged";
  IceRestart();
}

void ConferencePeerConnectionChannel::OnStreamError(
    const std::string& error_message) {
  std::shared_ptr<const ConferenceException> e(
      new ConferenceException(ConferenceException::kUnknown, error_message));
  std::shared_ptr<Stream> error_stream;
  if (published_stream_) {
    Unpublish(published_stream_, nullptr, nullptr);
    error_stream = published_stream_;
  }
  if (subscribed_stream_) {
    Unsubscribe(subscribed_stream_, nullptr, nullptr);
    error_stream = subscribed_stream_;
  }
  if (error_stream == nullptr) {
    RTC_DCHECK(false);
    return;
  }
  for (auto its = observers_.begin(); its != observers_.end(); ++its) {
    (*its).get().OnStreamError(error_stream, e);
  }
}
}
}
