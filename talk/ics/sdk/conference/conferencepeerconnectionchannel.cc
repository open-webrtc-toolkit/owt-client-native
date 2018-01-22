/*
 * Intel License
 */

#include <vector>
#include <thread>
#include <future>

#include "webrtc/rtc_base/logging.h"
#include "webrtc/rtc_base/task_queue.h"
#include "webrtc/media/base/videocommon.h"
#include "webrtc/media/base/videocapturer.h"

#include "talk/ics/sdk/base/functionalobserver.h"
#include "talk/ics/sdk/base/mediautils.h"
#include "talk/ics/sdk/base/peerconnectiondependencyfactory.h"
#include "talk/ics/sdk/conference/conferencepeerconnectionchannel.h"
#include "talk/ics/sdk/include/cpp/ics/conference/remotemixedstream.h"

namespace ics {
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
const int kMessageSeqBase = 1;

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
    std::shared_ptr<ConferenceSocketSignalingChannel> signaling_channel,
    std::shared_ptr<rtc::TaskQueue> event_queue)
    : PeerConnectionChannel(configuration),
      signaling_channel_(signaling_channel),
      session_id_(""),
      message_seq_(kMessageSeqBase),
      ice_restart_needed_(false),
      connected_(false),
      event_queue_(event_queue) {
  InitializePeerConnection();
  RTC_CHECK(signaling_channel_);
}

ConferencePeerConnectionChannel::~ConferencePeerConnectionChannel() {
  LOG(LS_INFO) << "Deconstruct conference peer connection channel";
  if (published_stream_)
    Unpublish(GetSessionId(), nullptr, nullptr);
  if (subscribed_stream_)
    Unsubscribe(GetSessionId(), nullptr, nullptr);
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
    rtc::scoped_refptr<MediaStreamInterface> stream) {
  LOG(LS_INFO) << "On add stream.";
  if (subscribed_stream_ != nullptr)
    subscribed_stream_->MediaStream(stream);
}

void ConferencePeerConnectionChannel::OnRemoveStream(
    rtc::scoped_refptr<MediaStreamInterface> stream) {}

void ConferencePeerConnectionChannel::OnDataChannel(
    rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) {}

void ConferencePeerConnectionChannel::OnRenegotiationNeeded() {}

void ConferencePeerConnectionChannel::OnIceConnectionChange(
    PeerConnectionInterface::IceConnectionState new_state) {
  LOG(LS_INFO) << "Ice connection state changed: " << new_state;
  if (new_state == PeerConnectionInterface::kIceConnectionConnected ||
      new_state == PeerConnectionInterface::kIceConnectionCompleted) {
    connected_ = true;
  } else if (new_state == PeerConnectionInterface::kIceConnectionClosed) {
    if (connected_) {
      OnStreamError(std::string("Stream connection closed unexpectedly."));
    }
    connected_ = false;
  } else {
    return;
  }
  // It's better to clean all callbacks to avoid fire them again. But callbacks
  // are run in task queue, so we cannot clean it here. Also, PostTaskAndReply
  // requires a reply queue which is not available at this time.
}

void ConferencePeerConnectionChannel::OnIceGatheringChange(
    PeerConnectionInterface::IceGatheringState new_state) {
  LOG(LS_INFO) << "Ice gathering state changed: " << new_state;
}

// TODO(jianlin): New signaling protocol defines candidate as
// a string instead of object. Need to double check with server
// side implementation before we switch to it.
void ConferencePeerConnectionChannel::OnIceCandidate(
    const webrtc::IceCandidateInterface* candidate) {
  LOG(LS_INFO) << "On ice candidate";
  string candidate_string;
  candidate->ToString(&candidate_string);
  candidate_string.insert(0, "a=");
  sio::message::ptr message = sio::object_message::create();
  message->get_map()["id"] = sio::string_message::create(session_id_);
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
  message->get_map()["signaling"] = sdp_message;
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
  message->get_map()["id"] = sio::string_message::create(session_id_);
  sio::message::ptr sdp_message = sio::object_message::create();
  sdp_message->get_map()["type"] = sio::string_message::create(desc->type());
  sdp_message->get_map()["sdp"] = sio::string_message::create(sdp);
  message->get_map()["signaling"] = sdp_message;
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
    event_queue_->PostTask([on_failure]() {
      std::unique_ptr<ConferenceException> e(new ConferenceException(
          ConferenceException::kUnknown, "Nullptr is not allowed."));
      on_failure(std::move(e));
    });
  }
  return false;
}

// Failure of publish will be handled here directly; while success needs
// conference client to construct the ConferencePublication instance,
// So we're not passing success callback here.
void ConferencePeerConnectionChannel::Publish(
    std::shared_ptr<LocalStream> stream,
    std::function<void(std::string)> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  LOG(LS_INFO) << "Publish a local stream.";
  published_stream_ = stream;
  if ((!CheckNullPointer((uintptr_t)stream.get(), on_failure)) ||
      (!CheckNullPointer((uintptr_t)stream->MediaStream(), on_failure))) {
    LOG(LS_INFO) << "Local stream cannot be nullptr.";
    return;
  }

  publish_success_callback_ = on_success;
  failure_callback_ = on_failure;

  media_constraints_.SetMandatory(
      webrtc::MediaConstraintsInterface::kOfferToReceiveAudio, false);
  media_constraints_.SetMandatory(
      webrtc::MediaConstraintsInterface::kOfferToReceiveVideo, false);

  sio::message::ptr options = sio::object_message::create();
  // type
  options->get_map()["type"] = sio::string_message::create("webrtc");
  // attributes
  sio::message::ptr attributes_ptr = sio::object_message::create();
  for (auto const& attr : stream->Attributes()) {
    attributes_ptr->get_map()[attr.first] =
        sio::string_message::create(attr.second);
  }
  options->get_map()[kStreamOptionAttributesKey] = attributes_ptr;

  // media
  sio::message::ptr media_ptr = sio::object_message::create();
  if (stream->MediaStream()->GetAudioTracks().size() == 0) {
    media_ptr->get_map()["audio"] = sio::bool_message::create(false);
  } else {
    sio::message::ptr audio_options = sio::object_message::create();
    if (stream->GetStreamDeviceType() ==
        ics::base::LocalStream::StreamDeviceType::kStreamDeviceTypeScreen) {
      audio_options->get_map()["source"] =
          sio::string_message::create("screen-cast");
    } else {
      audio_options->get_map()["source"] =
          sio::string_message::create("mic");
    }
    media_ptr->get_map()["audio"] = audio_options;
  }
  if (stream->MediaStream()->GetVideoTracks().size() == 0) {
    media_ptr->get_map()["video"] = sio::bool_message::create(false);
  } else {
    sio::message::ptr video_options = sio::object_message::create();
    if (stream->GetStreamDeviceType() ==
        ics::base::LocalStream::StreamDeviceType::kStreamDeviceTypeScreen) {
      video_options->get_map()["source"] =
          sio::string_message::create("screen-cast");
    } else {
      video_options->get_map()["source"] =
          sio::string_message::create("camera");
    }
    media_ptr->get_map()["video"] = video_options;
  }
  options->get_map()["media"] = media_ptr;
  SendPublishMessage(options, stream, on_failure);
}

void ConferencePeerConnectionChannel::Subscribe(
    std::shared_ptr<RemoteStream> stream,
    const SubscribeOptions& subscribe_options,
    std::function<void(std::string)> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  LOG(LS_INFO) << "Subscribe a remote stream. It has audio? "
               << stream->has_audio_ << ", has video? " << stream->has_video_;
  subscribed_stream_ = stream;
  if (!CheckNullPointer((uintptr_t)stream.get(), on_failure)) {
    LOG(LS_ERROR) << "Remote stream cannot be nullptr.";
    return;
  }
  if (subscribe_success_callback_) {
    if (on_failure) {
      event_queue_->PostTask([on_failure]() {
        std::unique_ptr<ConferenceException> e(new ConferenceException(
            ConferenceException::kUnknown, "Subscribing this stream."));
        on_failure(std::move(e));
      });
    }
  }
  subscribe_success_callback_ = on_success;
  failure_callback_ = on_failure;
  sio::message::ptr sio_options = sio::object_message::create();
  sio_options->get_map()["type"] = sio::string_message::create("webrtc");
  // For connection type equal to "webrtc", no connection object required.
  sio::message::ptr media_options = sio::object_message::create();
  if (stream->has_audio_) {
    sio::message::ptr audio_options = sio::object_message::create();
    audio_options->get_map()["from"] = sio::string_message::create(stream->Id());
    media_options->get_map()["audio"] = audio_options;
  } else {
    media_options->get_map()["audio"] = sio::bool_message::create(false);
  }

  // For webrtc, at present server does not support specifiying framerate/
  // keyFrameInterval. And for bitrate, we support "WantedBitrateMultiple".
  if (stream->has_video_) {
    sio::message::ptr video_options = sio::object_message::create();
    video_options->get_map()["from"] = sio::string_message::create(stream->Id());
    sio::message::ptr video_spec = sio::object_message::create();
    if (subscribe_options.resolution.width != 0 &&
        subscribe_options.resolution.height != 0) {
      sio::message::ptr resolution_options = sio::object_message::create();
      resolution_options->get_map()["width"] =
          sio::int_message::create(subscribe_options.resolution.width);
      resolution_options->get_map()["height"] =
          sio::int_message::create(subscribe_options.resolution.height);
      video_spec->get_map()["resolution"] = resolution_options;
    }
    std::string quality_level("x1.0");
    switch (subscribe_options.video_quality_level) {
      case SubscribeOptions::VideoQualityLevel::kStandard:
        break;
      case SubscribeOptions::VideoQualityLevel::kBestQuality:
        quality_level = "x1.4";
        break;
      case SubscribeOptions::VideoQualityLevel::kBetterQuality:
        quality_level = "x1.2";
        break;
      case SubscribeOptions::VideoQualityLevel::kBetterSpeed:
        quality_level = "x0.8";
        break;
      case SubscribeOptions::VideoQualityLevel::kBestSpeed:
        quality_level = "x0.6";
        break;
      default:
        RTC_NOTREACHED();
        break;
    }
    if (quality_level != "x1.0") {
      sio::message::ptr quality_options =
          sio::string_message::create(quality_level);
      video_spec->get_map()["bitrate"] = quality_options;
    }
    video_options->get_map()["parameters"] = video_spec;
    media_options->get_map()["video"] = video_options;
  } else {
    media_options->get_map()["video"] = sio::bool_message::create(false);
  }
  sio_options->get_map()["media"] = media_options;

  media_constraints_.SetMandatory(
      webrtc::MediaConstraintsInterface::kOfferToReceiveAudio,
      stream->has_audio_);
  media_constraints_.SetMandatory(
      webrtc::MediaConstraintsInterface::kOfferToReceiveVideo,
      stream->has_video_);
  signaling_channel_->SendInitializationMessage(
      sio_options, "", stream->Id(),
      [this](std::string session_id) {
        // Pre-set the session's ID.
        SetSessionId(session_id);
        CreateOffer();
      },
      on_failure);  // TODO: on_failure
}

void ConferencePeerConnectionChannel::Subscribe(
    std::shared_ptr<RemoteMixedStream> stream,
    const SubscribeOptions& subscribe_options,
    std::function<void(std::string)> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (subscribe_options.resolution.width != 0 ||
      subscribe_options.resolution.height != 0) {
    auto video_subscription_cap =
      (stream->SubscriptionCapabilities()).video_capabilities.resolutions;
    if (std::find_if(video_subscription_cap.begin(), video_subscription_cap.end(),
                     [&](const Resolution& format) {
                       return format == subscribe_options.resolution;
                     }) != video_subscription_cap.end()) {
      Subscribe(std::static_pointer_cast<RemoteStream>(stream),
                subscribe_options, on_success, on_failure);
    } else {
      LOG(LS_ERROR) << "Subscribe unsupported resolution.";
      if (on_failure != nullptr) {
        event_queue_->PostTask([on_failure]() {
          std::unique_ptr<ConferenceException> e(new ConferenceException(
              ConferenceException::kUnknown, "Unsupported resolution."));
          on_failure(std::move(e));
        });
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
    std::function<void(std::string)> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (subscribe_options.resolution.width != 0 ||
      subscribe_options.resolution.height != 0) {
    LOG(LS_ERROR) << "Screen sharing stream does not support resolution settings. "
                     "Please don't change resolution.";
    if (on_failure != nullptr) {
      event_queue_->PostTask([on_failure]() {
        std::unique_ptr<ConferenceException> e(new ConferenceException(
            ConferenceException::kUnknown,
            "Cannot specify resolution settings for screen sharing stream."));
        on_failure(std::move(e));
      });
      }
  } else {
    Subscribe(std::static_pointer_cast<RemoteStream>(stream),
                  subscribe_options, on_success, on_failure);
  }
}


void ConferencePeerConnectionChannel::Subscribe(
    std::shared_ptr<RemoteCameraStream> stream,
    const SubscribeOptions& subscribe_options,
    std::function<void(std::string)> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (subscribe_options.resolution.width != 0 ||
      subscribe_options.resolution.height != 0) {
    LOG(LS_ERROR) << "Camera stream does not support resolution settings. "
                     "Please don't change resolution.";
    if (on_failure != nullptr) {
      event_queue_->PostTask([on_failure]() {
        std::unique_ptr<ConferenceException> e(new ConferenceException(
            ConferenceException::kUnknown,
            "Cannot specify resolution settings for camera stream."));
        on_failure(std::move(e));
      });
      }
  } else {
    Subscribe(std::static_pointer_cast<RemoteStream>(stream),
                  subscribe_options, on_success, on_failure);
  }
}

void ConferencePeerConnectionChannel::Unpublish(
    const std::string& session_id,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (session_id != GetSessionId()) {
    LOG(LS_ERROR) << "Publication ID mismatch.";
    if (on_failure != nullptr) {
      event_queue_->PostTask([on_failure]() {
        std::unique_ptr<ConferenceException> e(
            new ConferenceException(ConferenceException::kUnknown,
                                    "Invalid stream to be unpublished."));
        on_failure(std::move(e));
      });
    }
    return;
  }

  connected_ = false;
  signaling_channel_->SendStreamEvent("unpublish", session_id,
                                      RunInEventQueue(on_success), on_failure);
  this->ClosePeerConnection();
}

void ConferencePeerConnectionChannel::Unsubscribe(
    const std::string& session_id,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (session_id != GetSessionId()) {
    LOG(LS_ERROR) << "Subscription ID mismatch.";
    if (on_failure != nullptr) {
      event_queue_->PostTask([on_failure]() {
        std::unique_ptr<ConferenceException> e(
            new ConferenceException(ConferenceException::kUnknown,
                                    "Invalid stream to be unsubscribed."));
        on_failure(std::move(e));
      });
    }
    return;
  }
  if (subscribe_success_callback_ != nullptr) {  // Subscribing
    if (on_failure != nullptr) {
      event_queue_->PostTask([on_failure]() {
        std::unique_ptr<ConferenceException> e(new ConferenceException(
            ConferenceException::kUnknown,
            "Cannot unsubscribe a stream during subscribing."));
        on_failure(std::move(e));
      });
    }
    return;
  }
  connected_ = false;
  signaling_channel_->SendStreamEvent("unsubscribe", session_id,
      RunInEventQueue(on_success), on_failure);
  this->ClosePeerConnection();
}

void ConferencePeerConnectionChannel::SendStreamControlMessage(
    const std::string& in_action,
    const std::string& out_action,
    const std::string& operation,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure)
    const {
  std::string action = "";
  if (published_stream_) {
      action = out_action;
      signaling_channel_->SendStreamControlMessage(
          session_id_, action, operation, on_success, on_failure);
  }
  else if (subscribed_stream_) {
      action = in_action;
      signaling_channel_->SendSubscriptionControlMessage(
          session_id_, action, operation, on_success, on_failure);
  }
  else
    RTC_DCHECK(false);
}

void ConferencePeerConnectionChannel::PlayAudioVideo(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  SendStreamControlMessage("av", "av", "play", on_success,
                           on_failure);
}

void ConferencePeerConnectionChannel::PauseAudioVideo(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  SendStreamControlMessage("av", "av", "pause", on_success,
                           on_failure);
}

void ConferencePeerConnectionChannel::PlayAudio(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  SendStreamControlMessage("audio", "audio", "play", on_success,
                           on_failure);
}

void ConferencePeerConnectionChannel::PauseAudio(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  SendStreamControlMessage("audio", "audio", "pause", on_success,
                           on_failure);
}

void ConferencePeerConnectionChannel::PlayVideo(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  SendStreamControlMessage("video", "video", "play", on_success,
                           on_failure);
}

void ConferencePeerConnectionChannel::PauseVideo(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  SendStreamControlMessage("video", "video", "pause", on_success,
                           on_failure);
}

void ConferencePeerConnectionChannel::Stop(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  LOG(LS_INFO) << "Stop session.";
}

void ConferencePeerConnectionChannel::GetConnectionStats(
    std::function<void(std::shared_ptr<ConnectionStats>)> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  if (!published_stream_ && !subscribed_stream_) {
    if (on_failure != nullptr) {
      event_queue_->PostTask([on_failure]() {
        std::unique_ptr<ConferenceException> e(
            new ConferenceException(ConferenceException::kUnknown,
                                    "No stream associated with the session"));
        on_failure(std::move(e));
      });
    }
    return;
  }
  if (subscribed_stream_) {
    scoped_refptr<FunctionalStatsObserver> observer = FunctionalStatsObserver::Create(on_success);
    GetStatsMessage* stats_message = new GetStatsMessage(
        observer.get(), subscribed_stream_->MediaStream(),
        webrtc::PeerConnectionInterface::kStatsOutputLevelStandard);
    pc_thread_->Post(RTC_FROM_HERE, this, kMessageTypeGetStats, stats_message);
  } else {
    scoped_refptr<FunctionalStatsObserver> observer = FunctionalStatsObserver::Create(on_success);
    GetStatsMessage* stats_message = new GetStatsMessage(
        observer.get(), published_stream_->MediaStream(),
        webrtc::PeerConnectionInterface::kStatsOutputLevelStandard);
    pc_thread_->Post(RTC_FROM_HERE, this, kMessageTypeGetStats, stats_message);
  }
}

void ConferencePeerConnectionChannel::OnSignalingMessage(
    sio::message::ptr message) {
  if (message == nullptr) {
    LOG(LS_INFO) << "Ignore empty signaling message";
    return;
  }

  if (message->get_flag() == sio::message::flag_string) {
    if (message->get_string() == "success") {
      // TODO(jianlin): remove this mix logic.
      if (published_stream_ != nullptr)
        signaling_channel_->SendStreamControlMessage(GetSessionId(), "common", "mix", nullptr, nullptr);
      std::weak_ptr<ConferencePeerConnectionChannel> weak_this =
            shared_from_this();
      if (publish_success_callback_) {
        event_queue_->PostTask([weak_this] {
              auto that = weak_this.lock();
              std::lock_guard<std::mutex> lock(that->callback_mutex_);
              if (!that || !that->publish_success_callback_)
                return;
              that->publish_success_callback_(that->GetSessionId());
              that->ResetCallbacks();
        });
      } else if (subscribe_success_callback_) {
        event_queue_->PostTask([weak_this] {
          auto that = weak_this.lock();
          std::lock_guard<std::mutex> lock(that->callback_mutex_);
          if (!that || !that->subscribe_success_callback_)
            return;
           that->subscribe_success_callback_(that->GetSessionId());
           that->ResetCallbacks();
        });
      }
      return;
    } else if (message->get_string() == "failure") {
      if (!connected_ && failure_callback_) {
        std::weak_ptr<ConferencePeerConnectionChannel> weak_this =
            shared_from_this();
        event_queue_->PostTask([weak_this] {
          auto that = weak_this.lock();
          std::lock_guard<std::mutex> lock(that->callback_mutex_);
          if (!that || !that->failure_callback_)
            return;
          std::unique_ptr<ConferenceException> e(new ConferenceException(
              ConferenceException::kUnknown, "MCU internal error during connection establishment."));
          that->failure_callback_(std::move(e));
          that->ResetCallbacks();
        });
      }
    }
    return;
  } else if (message->get_flag() != sio::message::flag_object) {
    LOG(LS_WARNING) << "Ignore invalid signaling message from MCU.";
    return;
  }
  // Since trickle ICE from MCU is not supported, we parse the message as
  // SOAC message, not Canddiate message.
  if (message->get_map().find("type") == message->get_map().end()) {
      LOG(LS_INFO) << "Ignore erizo message without type from MCU.";
      return;
  }

  if(message->get_map()["type"]->get_flag() != sio::message::flag_string
     || message->get_map()["sdp"] == nullptr
     || message->get_map()["sdp"]->get_flag() != sio::message::flag_string) {
    LOG(LS_ERROR) << "Invalid signaling message";
    return;
  }
  const std::string type = message->get_map()["type"]->get_string();
  LOG(LS_INFO) << "On signaling message: " << type;
  if (type == "answer") {
    const std::string sdp = message->get_map()["sdp"]->get_string();
    SetRemoteDescription(type, sdp);
  } else {
    LOG(LS_ERROR) << "Ignoring signaling message from server other than answer.";
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
  if (published_stream_ != nullptr) {
    //published_stream_->Id(id);
  }
  SetSessionId(id);
}

void ConferencePeerConnectionChannel::SetSessionId(const std::string& id) {
  LOG(LS_INFO) << "Setting session ID for current channel";
  session_id_ = id;
}

std::string ConferencePeerConnectionChannel::GetSessionId() const {
  return session_id_;
}

void ConferencePeerConnectionChannel::SendPublishMessage(
    sio::message::ptr options,
    std::shared_ptr<LocalStream> stream,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  signaling_channel_->SendInitializationMessage(
      options, stream->MediaStream()->label(), "",
      [stream, this](std::string session_id) {
        SetSessionId(session_id);
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
    Unpublish(GetSessionId(), nullptr, nullptr);
    error_stream = published_stream_;
  }
  if (subscribed_stream_) {
    Unsubscribe(GetSessionId(), nullptr, nullptr);
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

std::function<void()> ConferencePeerConnectionChannel::RunInEventQueue(
    std::function<void()> func) {
  if (!func)
    return nullptr;
  std::weak_ptr<ConferencePeerConnectionChannel> weak_this = shared_from_this();
  return [func, weak_this] {
    auto that = weak_this.lock();
    if (!that)
      return;
    that->event_queue_->PostTask([func] { func(); });
  };
}

void ConferencePeerConnectionChannel::ResetCallbacks() {
  publish_success_callback_ = nullptr;
  subscribe_success_callback_ = nullptr;
  failure_callback_ = nullptr;
}

void ConferencePeerConnectionChannel::ClosePeerConnection() {
  LOG(LS_INFO) << "Close peer connection.";
  RTC_CHECK(pc_thread_);
  pc_thread_->Send(RTC_FROM_HERE, this, kMessageTypeClosePeerConnection, nullptr);
}

}
}
