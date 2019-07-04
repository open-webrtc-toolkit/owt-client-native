// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "talk/owt/sdk/conference/conferencepeerconnectionchannel.h"
#include <future>
#include <thread>
#include <vector>
#include "talk/owt/sdk/base/functionalobserver.h"
#include "talk/owt/sdk/base/mediautils.h"
#include "talk/owt/sdk/base/peerconnectiondependencyfactory.h"
#include "talk/owt/sdk/include/cpp/owt/conference/remotemixedstream.h"
#include "webrtc/rtc_base/logging.h"
#include "webrtc/rtc_base/task_queue.h"
using namespace rtc;
namespace owt {
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
      ice_restart_needed_(false),
      connected_(false),
      sub_stream_added_(false),
      sub_server_ready_(false),
      event_queue_(event_queue) {
  InitializePeerConnection();
  RTC_CHECK(signaling_channel_);
}
ConferencePeerConnectionChannel::~ConferencePeerConnectionChannel() {
  RTC_LOG(LS_INFO) << "Deconstruct conference peer connection channel";
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
    RTC_LOG(LS_WARNING) << "Adding duplicate observer.";
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
  RTC_LOG(LS_INFO) << "Create offer.";
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
  RTC_LOG(LS_INFO) << "Post create offer";
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
  RTC_LOG(LS_INFO) << "ICE restart";
  RTC_DCHECK(SignalingState() ==
             PeerConnectionInterface::SignalingState::kStable);
  media_constraints_.SetMandatory(
      webrtc::MediaConstraintsInterface::kIceRestart, true);
  this->CreateOffer();
}
void ConferencePeerConnectionChannel::CreateAnswer() {
  RTC_LOG(LS_INFO) << "Create answer.";
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
  RTC_LOG(LS_INFO) << "Post create answer";
  pc_thread_->Post(RTC_FROM_HERE, this, kMessageTypeCreateAnswer,
                   message_observer);
}
void ConferencePeerConnectionChannel::OnSignalingChange(
    PeerConnectionInterface::SignalingState new_state) {
  RTC_LOG(LS_INFO) << "Signaling state changed: " << new_state;
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
  RTC_LOG(LS_INFO) << "On add stream.";
  if (subscribed_stream_ != nullptr)
    subscribed_stream_->MediaStream(stream);
  std::weak_ptr<ConferencePeerConnectionChannel> weak_this = shared_from_this();
  if (subscribe_success_callback_) {
    bool server_ready = false;
    {
      std::lock_guard<std::mutex> lock(sub_stream_added_mutex_);
      server_ready = sub_server_ready_;
      sub_stream_added_ = true;
      if (server_ready) {
        event_queue_->PostTask([weak_this] {
          auto that = weak_this.lock();
          std::lock_guard<std::mutex> lock(that->callback_mutex_);
          if (!that || !that->subscribe_success_callback_)
            return;
          that->subscribe_success_callback_(that->GetSessionId());
          that->ResetCallbacks();
        });
        sub_server_ready_ = false;
        sub_stream_added_ = false;
      }
    }
  }
}
void ConferencePeerConnectionChannel::OnRemoveStream(
    rtc::scoped_refptr<MediaStreamInterface> stream) {}
void ConferencePeerConnectionChannel::OnDataChannel(
    rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) {}
void ConferencePeerConnectionChannel::OnRenegotiationNeeded() {}
void ConferencePeerConnectionChannel::OnIceConnectionChange(
    PeerConnectionInterface::IceConnectionState new_state) {
  RTC_LOG(LS_INFO) << "Ice connection state changed: " << new_state;
  if (new_state == PeerConnectionInterface::kIceConnectionConnected ||
      new_state == PeerConnectionInterface::kIceConnectionCompleted) {
    connected_ = true;
  } else if (new_state == PeerConnectionInterface::kIceConnectionFailed) {
    // TODO(jianlin): Change trigger condition back to kIceConnectionClosed
    // once MCU re-enables IceRestart and client supports it as well.
    if (connected_) {
      OnStreamError(std::string("Stream ICE connection failed."));
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
  RTC_LOG(LS_INFO) << "Ice gathering state changed: " << new_state;
}
// TODO(jianlin): New signaling protocol defines candidate as
// a string instead of object. Need to double check with server
// side implementation before we switch to it.
void ConferencePeerConnectionChannel::OnIceCandidate(
    const webrtc::IceCandidateInterface* candidate) {
  RTC_LOG(LS_INFO) << "On ice candidate";
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
void ConferencePeerConnectionChannel::OnIceCandidatesRemoved(
    const std::vector<cricket::Candidate>& candidates) {
  RTC_LOG(LS_INFO) << "On ice candidate removed";
  if (candidates.empty())
    return;
  sio::message::ptr message = sio::object_message::create();
  message->get_map()["id"] = sio::string_message::create(session_id_);
  sio::message::ptr remove_candidates_msg = sio::object_message::create();
  remove_candidates_msg->get_map()["type"] =
      sio::string_message::create("removed-candidates");
  sio::message::ptr removed_candidates = sio::array_message::create();
  for (auto candidate : candidates) {
    std::string candidate_string = candidate.ToString();
    candidate_string.insert(0, "a=");
    sio::message::ptr current_candidate = sio::object_message::create();
    current_candidate->get_map()["candidate"] =
        sio::string_message::create(candidate_string);
    // jianlin: Native stack does not pop sdpMid & sdpMLineIndex to observer.
    // Maybe need to create a hash table to map candidate id to sdpMid/sdpMlineIndex
    // in OnIceCandidate().
    removed_candidates->get_vector().push_back(current_candidate);
  }
  remove_candidates_msg->get_map()["candidates"] = removed_candidates;
  message->get_map()["signaling"] = remove_candidates_msg;
  if (signaling_channel_) {
    signaling_channel_->SendSdp(message, nullptr, nullptr);
  }
}
void ConferencePeerConnectionChannel::OnCreateSessionDescriptionSuccess(
    webrtc::SessionDescriptionInterface* desc) {
  RTC_LOG(LS_INFO) << "Create sdp success.";
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
  RTC_LOG(LS_INFO) << "Post set local desc";
  pc_thread_->Post(RTC_FROM_HERE, this, kMessageTypeSetLocalDescription, msg);
}
void ConferencePeerConnectionChannel::OnCreateSessionDescriptionFailure(
    const std::string& error) {
  RTC_LOG(LS_INFO) << "Create sdp failed.";
}
void ConferencePeerConnectionChannel::OnSetLocalSessionDescriptionSuccess() {
  RTC_LOG(LS_INFO) << "Set local sdp success.";
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
  RTC_LOG(LS_INFO) << "Set local sdp failed.";
  if (failure_callback_) {
    std::unique_ptr<Exception> e(new Exception(
        ExceptionType::kConferenceUnknown, "Failed to set local description."));
    failure_callback_(std::move(e));
    ResetCallbacks();
  }
  OnStreamError(std::string("Failed to set local description."));
}
void ConferencePeerConnectionChannel::OnSetRemoteSessionDescriptionSuccess() {
  PeerConnectionChannel::OnSetRemoteSessionDescriptionSuccess();
}
void ConferencePeerConnectionChannel::OnSetRemoteSessionDescriptionFailure(
    const std::string& error) {
  RTC_LOG(LS_INFO) << "Set remote sdp failed.";
  if (failure_callback_) {
    std::unique_ptr<Exception> e(new Exception(
        ExceptionType::kConferenceUnknown, "Fail to set remote description."));
    failure_callback_(std::move(e));
    ResetCallbacks();
  }
  OnStreamError(std::string("Failed to set remote description."));
}
void ConferencePeerConnectionChannel::SetRemoteDescription(
    const std::string& type,
    const std::string& sdp) {
  webrtc::SessionDescriptionInterface* desc(webrtc::CreateSessionDescription(
      "answer", sdp,
      nullptr));  // TODO(jianjun): change answer to type.toLowerCase.
  if (!desc) {
    RTC_LOG(LS_ERROR) << "Failed to create session description.";
    return;
  }
  scoped_refptr<FunctionalSetSessionDescriptionObserver> observer =
      FunctionalSetSessionDescriptionObserver::Create(
          std::bind(&ConferencePeerConnectionChannel::
                        OnSetRemoteSessionDescriptionSuccess,
                    this),
          std::bind(&ConferencePeerConnectionChannel::
                        OnSetRemoteSessionDescriptionFailure,
                    this, std::placeholders::_1));
  SetSessionDescriptionMessage* msg =
      new SetSessionDescriptionMessage(observer.get(), desc);
  RTC_LOG(LS_INFO) << "Post set remote desc";
  pc_thread_->Post(RTC_FROM_HERE, this, kMessageTypeSetRemoteDescription, msg);
}
bool ConferencePeerConnectionChannel::CheckNullPointer(
    uintptr_t pointer,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  if (pointer)
    return true;
  if (on_failure != nullptr) {
    event_queue_->PostTask([on_failure]() {
      std::unique_ptr<Exception> e(new Exception(
          ExceptionType::kConferenceUnknown, "Nullptr is not allowed."));
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
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  RTC_LOG(LS_INFO) << "Publish a local stream.";
  published_stream_ = stream;
  if ((!CheckNullPointer((uintptr_t)stream.get(), on_failure)) ||
      (!CheckNullPointer((uintptr_t)stream->MediaStream(), on_failure))) {
    RTC_LOG(LS_INFO) << "Local stream cannot be nullptr.";
  }
  if (isMediaStreamEnded(stream->MediaStream())) {
    if (on_failure != nullptr) {
      event_queue_->PostTask([on_failure]() {
        std::unique_ptr<Exception> e(new Exception(
            ExceptionType::kConferenceUnknown, "Cannot publish ended stream."));
        on_failure(std::move(e));
      });
    }
    return;
  }
  publish_success_callback_ = on_success;
  failure_callback_ = on_failure;
  offer_answer_options_.offer_to_receive_audio = false;
  offer_answer_options_.offer_to_receive_video = false;
  sio::message::ptr options = sio::object_message::create();
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
    if (stream->Source().audio == owt::base::AudioSourceInfo::kScreenCast) {
      audio_options->get_map()["source"] =
          sio::string_message::create("screen-cast");
    } else {
      audio_options->get_map()["source"] = sio::string_message::create("mic");
    }
    media_ptr->get_map()["audio"] = audio_options;
  }
  if (stream->MediaStream()->GetVideoTracks().size() == 0) {
    media_ptr->get_map()["video"] = sio::bool_message::create(false);
  } else {
    sio::message::ptr video_options = sio::object_message::create();
    if (stream->Source().video == owt::base::VideoSourceInfo::kScreenCast) {
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
static bool SubOptionAllowed(
    const SubscribeOptions& subscribe_options,
    const SubscriptionCapabilities& subscription_caps) {
  // TODO: Audio sub constraints are currently not checked as spec only
  // specifies codec, though signaling allows specifying sampleRate and channel
  // num.
  bool result = true;
  if (subscribe_options.video.resolution.width != 0 &&
      subscribe_options.video.resolution.height != 0) {
    result = false;
    if (std::find_if(subscription_caps.video.resolutions.begin(),
                     subscription_caps.video.resolutions.end(),
                     [&](const Resolution& format) {
                       return format == subscribe_options.video.resolution;
                     }) != subscription_caps.video.resolutions.end()) {
      result = true;
    }
  }
  if (subscribe_options.video.frameRate != 0) {
    result = false;
    if (std::find_if(subscription_caps.video.frame_rates.begin(),
                     subscription_caps.video.frame_rates.end(),
                     [&](const double& format) {
                       return format == subscribe_options.video.frameRate;
                     }) != subscription_caps.video.frame_rates.end()) {
      result = true;
    }
  }
  if (subscribe_options.video.keyFrameInterval != 0) {
    result = false;
    if (std::find_if(subscription_caps.video.keyframe_intervals.begin(),
                     subscription_caps.video.keyframe_intervals.end(),
                     [&](const unsigned long& format) {
                       return format ==
                              subscribe_options.video.keyFrameInterval;
                     }) != subscription_caps.video.keyframe_intervals.end()) {
      result = true;
    }
  }
  if (subscribe_options.video.bitrateMultiplier != 0) {
    result = false;
    if (std::find_if(subscription_caps.video.bitrate_multipliers.begin(),
                     subscription_caps.video.bitrate_multipliers.end(),
                     [&](const double& format) {
                       return format ==
                              subscribe_options.video.bitrateMultiplier;
                     }) != subscription_caps.video.bitrate_multipliers.end()) {
      result = true;
    }
  }
  return result;
}
void ConferencePeerConnectionChannel::Subscribe(
    std::shared_ptr<RemoteStream> stream,
    const SubscribeOptions& subscribe_options,
    std::function<void(std::string)> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  RTC_LOG(LS_INFO) << "Subscribe a remote stream. It has audio? "
                   << stream->has_audio_ << ", has video? "
                   << stream->has_video_;
  if (!SubOptionAllowed(subscribe_options, stream->Capabilities())) {
    RTC_LOG(LS_ERROR)
        << "Subscribe option mismatch with stream subcription capabilities.";
    if (on_failure != nullptr) {
      event_queue_->PostTask([on_failure]() {
        std::unique_ptr<Exception> e(
            new Exception(ExceptionType::kConferenceUnknown,
                          "Unsupported subscribe option."));
        on_failure(std::move(e));
      });
    }
    return;
  }
  subscribed_stream_ = stream;
  if (!CheckNullPointer((uintptr_t)stream.get(), on_failure)) {
    RTC_LOG(LS_ERROR) << "Remote stream cannot be nullptr.";
    return;
  }
  if (subscribe_success_callback_) {
    if (on_failure) {
      event_queue_->PostTask([on_failure]() {
        std::unique_ptr<Exception> e(new Exception(
            ExceptionType::kConferenceUnknown, "Subscribing this stream."));
        on_failure(std::move(e));
      });
    }
  }
  subscribe_success_callback_ = on_success;
  failure_callback_ = on_failure;
  sio::message::ptr sio_options = sio::object_message::create();
  sio::message::ptr media_options = sio::object_message::create();
  if (stream->has_audio_ && !subscribe_options.audio.disabled) {
    sio::message::ptr audio_options = sio::object_message::create();
    audio_options->get_map()["from"] =
        sio::string_message::create(stream->Id());
    media_options->get_map()["audio"] = audio_options;
  } else {
    media_options->get_map()["audio"] = sio::bool_message::create(false);
  }
  if (stream->has_video_ && !subscribe_options.video.disabled) {
    sio::message::ptr video_options = sio::object_message::create();
    video_options->get_map()["from"] =
        sio::string_message::create(stream->Id());
    sio::message::ptr video_spec = sio::object_message::create();
    sio::message::ptr resolution_options = sio::object_message::create();
    if (subscribe_options.video.resolution.width != 0 &&
        subscribe_options.video.resolution.height != 0) {
      resolution_options->get_map()["width"] =
          sio::int_message::create(subscribe_options.video.resolution.width);
      resolution_options->get_map()["height"] =
          sio::int_message::create(subscribe_options.video.resolution.height);
      video_spec->get_map()["resolution"] = resolution_options;
    }
    // If bitrateMultiplier is not specified, do not include it in video spec.
    std::string quality_level("x1.0");
    if (subscribe_options.video.bitrateMultiplier != 0) {
      quality_level =
          "x" + std::to_string(subscribe_options.video.bitrateMultiplier)
                    .substr(0, 3);
    }
    if (quality_level != "x1.0") {
      sio::message::ptr quality_options =
          sio::string_message::create(quality_level);
      video_spec->get_map()["bitrate"] = quality_options;
    }
    if (subscribe_options.video.keyFrameInterval != 0) {
      video_spec->get_map()["keyFrameInterval"] =
          sio::int_message::create(subscribe_options.video.keyFrameInterval);
    }
    if (subscribe_options.video.frameRate != 0) {
      video_spec->get_map()["framerate"] =
          sio::int_message::create(subscribe_options.video.frameRate);
    }
    video_options->get_map()["parameters"] = video_spec;
    media_options->get_map()["video"] = video_options;
  } else {
    media_options->get_map()["video"] = sio::bool_message::create(false);
  }
  sio_options->get_map()["media"] = media_options;
  offer_answer_options_.offer_to_receive_audio =
      stream->has_audio_ && !subscribe_options.audio.disabled;
  offer_answer_options_.offer_to_receive_video =
      stream->has_video_ && !subscribe_options.video.disabled;
  signaling_channel_->SendInitializationMessage(
      sio_options, "", stream->Id(),
      [this](std::string session_id) {
        // Pre-set the session's ID.
        SetSessionId(session_id);
        CreateOffer();
      },
      on_failure);  // TODO: on_failure
}
void ConferencePeerConnectionChannel::Unpublish(
    const std::string& session_id,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  if (session_id != GetSessionId()) {
    RTC_LOG(LS_ERROR) << "Publication ID mismatch.";
    if (on_failure != nullptr) {
      event_queue_->PostTask([on_failure]() {
        std::unique_ptr<Exception> e(
            new Exception(ExceptionType::kConferenceUnknown,
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
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  if (session_id != GetSessionId()) {
    RTC_LOG(LS_ERROR) << "Subscription ID mismatch.";
    if (on_failure != nullptr) {
      event_queue_->PostTask([on_failure]() {
        std::unique_ptr<Exception> e(
            new Exception(ExceptionType::kConferenceUnknown,
                          "Invalid stream to be unsubscribed."));
        on_failure(std::move(e));
      });
    }
    return;
  }
  if (subscribe_success_callback_ != nullptr) {  // Subscribing
    if (on_failure != nullptr) {
      event_queue_->PostTask([on_failure]() {
        std::unique_ptr<Exception> e(
            new Exception(ExceptionType::kConferenceUnknown,
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
    std::function<void(std::unique_ptr<Exception>)> on_failure) const {
  std::string action = "";
  if (published_stream_) {
    action = out_action;
    signaling_channel_->SendStreamControlMessage(session_id_, action, operation,
                                                 on_success, on_failure);
  } else if (subscribed_stream_) {
    action = in_action;
    signaling_channel_->SendSubscriptionControlMessage(
        session_id_, action, operation, on_success, on_failure);
  } else
    RTC_DCHECK(false);
}
void ConferencePeerConnectionChannel::PlayAudioVideo(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  SendStreamControlMessage("av", "av", "play", on_success, on_failure);
}
void ConferencePeerConnectionChannel::PauseAudioVideo(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  SendStreamControlMessage("av", "av", "pause", on_success, on_failure);
}
void ConferencePeerConnectionChannel::PlayAudio(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  SendStreamControlMessage("audio", "audio", "play", on_success, on_failure);
}
void ConferencePeerConnectionChannel::PauseAudio(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  SendStreamControlMessage("audio", "audio", "pause", on_success, on_failure);
}
void ConferencePeerConnectionChannel::PlayVideo(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  SendStreamControlMessage("video", "video", "play", on_success, on_failure);
}
void ConferencePeerConnectionChannel::PauseVideo(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  SendStreamControlMessage("video", "video", "pause", on_success, on_failure);
}
void ConferencePeerConnectionChannel::Stop(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  RTC_LOG(LS_INFO) << "Stop session.";
}
void ConferencePeerConnectionChannel::GetConnectionStats(
    std::function<void(std::shared_ptr<ConnectionStats>)> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  if (!published_stream_ && !subscribed_stream_) {
    if (on_failure != nullptr) {
      event_queue_->PostTask([on_failure]() {
        std::unique_ptr<Exception> e(
            new Exception(ExceptionType::kConferenceUnknown,
                          "No stream associated with the session"));
        on_failure(std::move(e));
      });
    }
    return;
  }
  if (subscribed_stream_) {
    scoped_refptr<FunctionalStatsObserver> observer =
        FunctionalStatsObserver::Create(on_success);
    GetStatsMessage* stats_message = new GetStatsMessage(
        observer.get(), subscribed_stream_->MediaStream(),
        webrtc::PeerConnectionInterface::kStatsOutputLevelStandard);
    pc_thread_->Post(RTC_FROM_HERE, this, kMessageTypeGetStats, stats_message);
  } else {
    scoped_refptr<FunctionalStatsObserver> observer =
        FunctionalStatsObserver::Create(on_success);
    GetStatsMessage* stats_message = new GetStatsMessage(
        observer.get(), published_stream_->MediaStream(),
        webrtc::PeerConnectionInterface::kStatsOutputLevelStandard);
    pc_thread_->Post(RTC_FROM_HERE, this, kMessageTypeGetStats, stats_message);
  }
}
void ConferencePeerConnectionChannel::GetStats(
    std::function<void(const webrtc::StatsReports& reports)> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  if (!on_success) {
    return;
  }
  scoped_refptr<FunctionalNativeStatsObserver> observer =
      FunctionalNativeStatsObserver::Create(on_success);
  GetNativeStatsMessage* stats_message = new GetNativeStatsMessage(
      observer.get(), nullptr,
      webrtc::PeerConnectionInterface::kStatsOutputLevelStandard);
  pc_thread_->Post(RTC_FROM_HERE, this, kMessageTypeGetStats, stats_message);
}
void ConferencePeerConnectionChannel::OnSignalingMessage(
    sio::message::ptr message) {
  if (message == nullptr) {
    RTC_LOG(LS_INFO) << "Ignore empty signaling message";
    return;
  }
  if (message->get_flag() == sio::message::flag_string) {
    if (message->get_string() == "success") {
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
        bool stream_added = false;
        {
          std::lock_guard<std::mutex> lock(sub_stream_added_mutex_);
          stream_added = sub_stream_added_;
          sub_server_ready_ = true;
          if (stream_added) {
            event_queue_->PostTask([weak_this] {
              auto that = weak_this.lock();
              std::lock_guard<std::mutex> lock(that->callback_mutex_);
              if (!that || !that->subscribe_success_callback_)
                return;
              that->subscribe_success_callback_(that->GetSessionId());
              that->ResetCallbacks();
            });
            sub_server_ready_ = false;
            sub_stream_added_ = false;
          }
        }
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
          std::unique_ptr<Exception> e(new Exception(
              ExceptionType::kConferenceUnknown,
              "MCU internal error during connection establishment."));
          that->failure_callback_(std::move(e));
          that->ResetCallbacks();
        });
      }
    }
    return;
  } else if (message->get_flag() != sio::message::flag_object) {
    RTC_LOG(LS_WARNING) << "Ignore invalid signaling message from MCU.";
    return;
  }
  // Since trickle ICE from MCU is not supported, we parse the message as
  // SOAC message, not Canddiate message.
  if (message->get_map().find("type") == message->get_map().end()) {
    RTC_LOG(LS_INFO) << "Ignore erizo message without type from MCU.";
    return;
  }
  if (message->get_map()["type"]->get_flag() != sio::message::flag_string ||
      message->get_map()["sdp"] == nullptr ||
      message->get_map()["sdp"]->get_flag() != sio::message::flag_string) {
    RTC_LOG(LS_ERROR) << "Invalid signaling message";
    return;
  }
  const std::string type = message->get_map()["type"]->get_string();
  RTC_LOG(LS_INFO) << "On signaling message: " << type;
  if (type == "answer") {
    const std::string sdp = message->get_map()["sdp"]->get_string();
    SetRemoteDescription(type, sdp);
  } else {
    RTC_LOG(LS_ERROR)
        << "Ignoring signaling message from server other than answer.";
  }
}
void ConferencePeerConnectionChannel::DrainIceCandidates() {
  std::lock_guard<std::mutex> lock(candidates_mutex_);
  for (auto it = ice_candidates_.begin(); it != ice_candidates_.end(); ++it) {
    signaling_channel_->SendSdp(*it, nullptr, nullptr);
  }
  ice_candidates_.clear();
}
std::string ConferencePeerConnectionChannel::GetSubStreamId() {
  if (subscribed_stream_) {
    return subscribed_stream_->Id();
  } else {
    return "";
  }
}
void ConferencePeerConnectionChannel::SetSessionId(const std::string& id) {
  RTC_LOG(LS_INFO) << "Setting session ID for current channel";
  session_id_ = id;
}
std::string ConferencePeerConnectionChannel::GetSessionId() const {
  return session_id_;
}
void ConferencePeerConnectionChannel::SendPublishMessage(
    sio::message::ptr options,
    std::shared_ptr<LocalStream> stream,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  signaling_channel_->SendInitializationMessage(
      options, stream->MediaStream()->id(), "",
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
void ConferencePeerConnectionChannel::OnNetworksChanged() {
  RTC_LOG(LS_INFO) << "ConferencePeerConnectionChannel::OnNetworksChanged";
}
void ConferencePeerConnectionChannel::OnStreamError(
    const std::string& error_message) {
  std::shared_ptr<const Exception> e(
      new Exception(ExceptionType::kConferenceUnknown, error_message));
  std::shared_ptr<Stream> error_stream;
  for (auto its = observers_.begin(); its != observers_.end(); ++its) {
    RTC_LOG(LS_INFO) << "On stream error.";
    (*its).get().OnStreamError(error_stream, e);
  }
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
  RTC_LOG(LS_INFO) << "Close peer connection.";
  RTC_CHECK(pc_thread_);
  pc_thread_->Post(RTC_FROM_HERE, this, kMessageTypeClosePeerConnection,
                   nullptr);
}
bool ConferencePeerConnectionChannel::isMediaStreamEnded(
    MediaStreamInterface* stream) const {
  RTC_CHECK(stream);
  for (auto track : stream->GetAudioTracks()) {
    if (track->state() == webrtc::AudioTrackInterface::TrackState::kLive) {
      return false;
    }
  }
  for (auto track : stream->GetVideoTracks()) {
    if (track->state() == webrtc::VideoTrackInterface::TrackState::kLive) {
      return false;
    }
  }
  return true;
}
}  // namespace conference
}  // namespace owt
