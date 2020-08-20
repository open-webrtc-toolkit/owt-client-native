// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "talk/owt/sdk/conference/conferencewebtransportchannel.h"
#include <future>
#include <string>
#include <thread>
#include <vector>
#include "talk/owt/sdk/base/functionalobserver.h"
#include "talk/owt/sdk/base/mediautils.h"
#include "talk/owt/sdk/include/cpp/owt/base/globalconfiguration.h"
#include "webrtc/rtc_base/checks.h"
#include "webrtc/rtc_base/logging.h"
#include "owt/quic/quic_transport_client_interface.h"
#include "owt/quic/quic_transport_factory.h"
#include "owt/quic/quic_transport_stream_interface.h"

using namespace rtc;
namespace owt {
namespace conference {
using std::string;

// Stream option member key
const std::string kStreamOptionStreamIdKey = "streamId";
const std::string kStreamOptionStateKey = "state";
const std::string kStreamOptionDataKey = "type";
const std::string kStreamOptionWebTransportKey = "data";
const std::string kStreamOptionScreenKey = "screen";
const std::string kStreamOptionAttributesKey = "attributes";

ConferenceWebTransportChannel::ConferenceWebTransportChannel(
    const std::string& url,
    std::shared_ptr<ConferenceSocketSignalingChannel> signaling_channel,
    std::shared_ptr<rtc::TaskQueue> event_queue)
    : signaling_channel_(signaling_channel),
      session_id_(""),
      connected_(false),
      sub_stream_added_(false),
      sub_server_ready_(false),
      event_queue_(event_queue),
      url_(url) {
  quic_transport_factory_.reset(owt::quic::QuicTransportFactory::Create());
  quic_client_connected_ = false;
  RTC_CHECK(signaling_channel_);
}

ConferenceWebTransportChannel::~ConferenceWebTransportChannel() {
  RTC_LOG(LS_INFO) << "Deconstruct conference WebTransport channel";
  if (published_stream_)
    Unpublish(GetSessionId(), nullptr, nullptr);
  if (subscribed_stream_)
    Unsubscribe(GetSessionId(), nullptr, nullptr);
}

void ConferenceWebTransportChannel::Connect() {
  std::vector<owt::base::cert_fingerprint_t> trusted_fingerprints =
      GlobalConfiguration::GetTrustedQuicCertificateFingerPrints();
  size_t cert_fingerprint_size = trusted_fingerprints.size();

  owt::quic::QuicTransportClientInterface::Parameters quic_params;
  memset(&quic_params, 0, sizeof(quic_params));
  if (cert_fingerprint_size > 0) {
    quic_params.server_certificate_fingerprints =
        new owt::quic::CertificateFingerprint[cert_fingerprint_size];
    quic_params.server_certificate_fingerprints_length = cert_fingerprint_size;
  }
  int fingerprint_idx = 0;
  for (auto fingerprint : trusted_fingerprints) {
    quic_params.server_certificate_fingerprints[fingerprint_idx] =
        new char[QUIC_CERT_FINGERPRINT_SIZE];
    memcpy(&quic_params.server_certificate_fingerprints[fingerprint_idx],
           fingerprint.data(), fingerprint.size());
  }
  quic_transport_client = quic_transport_factory_->CreateQuicTransportClient(url_.c_str(), quic_params);
  if (quic_transport_client_.get()) {
    quic_transport_client_->SetVistor(this);
    // Async. Connect status will be notified through visitor.
    quic_transport_client_->Connect();
  }
}

void ConferenceWebTransportChannel::AddObserver(
    ConferenceWebTransportChannelObserver* observer) {
  RTC_CHECK(observer);
  observer_ = observer;
}
void ConferenceWebTransportChannel::RemoveObserver(
    ConferenceWebTransportChannelObserver& observer) {
  observer_ = nullptr;
}

bool ConferenceWebTransportChannel::CheckNullPointer(
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
void ConferenceWebTransportChannel::Publish(
    std::shared_ptr<LocalStream> stream,
    std::function<void(std::string, std::string)> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  RTC_LOG(LS_INFO) << "Publish a local stream.";
  published_stream_ = stream;
  if ((!CheckNullPointer((uintptr_t)stream.get(), on_failure)) ||
      (!CheckNullPointer((uintptr_t)stream->QuicStream(), on_failure))) {
    RTC_LOG(LS_INFO) << "Local stream cannot be nullptr.";
  }
  publish_success_callback_ = on_success;
  failure_callback_ = on_failure;
  sio::message::ptr options = sio::object_message::create();
  // attributes
  sio::message::ptr attributes_ptr = sio::object_message::create();
  for (auto const& attr : stream->Attributes()) {
    attributes_ptr->get_map()[attr.first] =
        sio::string_message::create(attr.second);
  }
  options->get_map()[kStreamOptionAttributesKey] = attributes_ptr;
  // media object should be null for data stream.
  sio::message::ptr media_ptr = sio::object_message::create();
  options->get_map()["media"] = media_ptr;
  options->get_map()["data"] = sio::bool_message::create(true);
  sio::message::ptr transport_ptr = sio::object_message::create();
  transport_ptr->get_map()["type"] = sio::string_message::create("quic");
  transport_ptr->get_map()["id"] = sio::null_message::create();
  SendPublishMessage(options, stream, on_failure);
}

// For WebTransport streams we will not check the subscription capabilities,
// as it does not contain any optional settings.
static bool SubOptionAllowed(
    const SubscribeOptions& subscribe_options,
    const PublicationSettings& publication_settings) {
  if (subscribe_options.data.enabled && publication_settings.data.enabled)
    return true;
  return false;
}
void ConferenceWebTransportChannel::Subscribe(
    std::shared_ptr<RemoteStream> stream,
    const SubscribeOptions& subscribe_options,
    std::function<void(std::string)> on_success,
    std::function<void(std::unique_ptr<owt::base::Exception>)> on_failure) {
  RTC_LOG(LS_INFO) << "Subscribe a remote stream. It supports data? "
                   << stream->DataEnabled();
  if (!SubOptionAllowed(subscribe_options, stream->Settings())) {
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
    if (subscribe_options.video.rid != "") {
      video_options->get_map()["simulcastRid"] =
          sio::string_message::create(subscribe_options.video.rid);
    }
    media_options->get_map()["video"] = video_options;
  } else {
    media_options->get_map()["video"] = sio::bool_message::create(false);
  }
  sio_options->get_map()["media"] = media_options;
  if (stream->has_audio_ && !subscribe_options.audio.disabled) {
    webrtc::RtpTransceiverInit transceiver_init;
    transceiver_init.direction = webrtc::RtpTransceiverDirection::kRecvOnly;
    AddTransceiver(cricket::MediaType::MEDIA_TYPE_AUDIO, transceiver_init);
  }
  if (stream->has_video_ && !subscribe_options.video.disabled) {
    webrtc::RtpTransceiverInit transceiver_init;
    transceiver_init.direction = webrtc::RtpTransceiverDirection::kRecvOnly;
    AddTransceiver(cricket::MediaType::MEDIA_TYPE_VIDEO, transceiver_init);
  }
  signaling_channel_->SendInitializationMessage(
      sio_options, "", stream->Id(),
      [this](std::string session_id) {
        // Pre-set the session's ID.
        SetSessionId(session_id);
      },
      on_failure);  // TODO: on_failure
}
void ConferencWebTransportChannel::Unpublish(
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
}
void ConferenceWebTransportChannel::Unsubscribe(
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
}
void ConferenceWebTransportChannel::SendStreamControlMessage(
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

// TODO: the webtransport channel implementation should not depend
// on specific signaling protocol such as Socket.IO.
void ConferencWebTransportChannel::OnSignalingMessage(
    sio::message::ptr message) {
  if (message == nullptr) {
    RTC_LOG(LS_INFO) << "Ignore empty signaling message";
    return;
  }
  if (message->get_flag() == sio::message::flag_string) {
    if (message->get_string() == "success") {
      std::weak_ptr<ConferenceWebTransportChannel> weak_this =
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
        std::weak_ptr<ConferenceWebTransportChannel> weak_this =
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

std::string ConferenceWebTransportChannel::GetSubStreamId() {
  if (subscribed_stream_) {
    return subscribed_stream_->Id();
  } else {
    return "";
  }
}

void ConferenceWebTransportChannel::SetSessionId(const std::string& id) {
  RTC_LOG(LS_INFO) << "Setting session ID for current channel";
  session_id_ = id;
}

std::string ConferenceWebTransportChannel::GetSessionId() const {
  return session_id_;
}

void ConferenceWebTransportChannel::SendPublishMessage(
    sio::message::ptr options,
    std::shared_ptr<owt::base::LocalStream> stream,
    std::function<void(std::unique_ptr<owt::base::Exception>)> on_failure) {
  std::weak_ptr<ConferenceWebTransportChannel> weak_this = shared_from_this();
  signaling_channel_->SendInitializationMessage(
      options, stream->MediaStream()->id(), "",
      [stream, weak_this, on_failure](std::string session_id, std::string publication_id) {
        auto that = weak_this.lock();
        if (!that)
        SetSessionId(session_id);
      },
      on_failure);
}

void ConferenceWebTransportChannel::OnStreamError(
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

std::function<void()> ConferenceWebTransportChannel::RunInEventQueue(
    std::function<void()> func) {
  if (!func)
    return nullptr;
  std::weak_ptr<ConferenceWebTransportChannel> weak_this = shared_from_this();
  return [func, weak_this] {
    auto that = weak_this.lock();
    if (!that)
      return;
    that->event_queue_->PostTask([func] { func(); });
  };
}
void ConferenceWebTransportChannel::ResetCallbacks() {
  publish_success_callback_ = nullptr;
  subscribe_success_callback_ = nullptr;
  failure_callback_ = nullptr;
}

void ConferenceWebTransportChannel::OnConnected() {
  RTC_LOG(LS_INFO) << "Quic client connected.";
  quic_client_connected_ = true;
  if (observer_) {
    observer_->OnConnected();
  }
}

void ConferenceWebTransportChannel::OnConnectionFailed() {
  RTC_LOG(LS_INFO) << "Quic client disconnected.";
  quic_client_connected_ = false;
  if (observer_) {
    observer_->OnConnectionFailed();
  }
}

void ConferenceWebTransportChannel::OnIncomingStream(
    owt::quic::QuicTransportStreamInterface* stream) {
  // Suppose this should not get invoked.
  RTC_LOG(LS_INFO) << "Incoming quic stream.";
  if (observer_) {
    observer_->OnIncomingStream(stream);
  }
}

void ConferenceWebTransportChannel::CreateSendStream(
    std::function<void(std::shared_ptr<owt::base::WritableStream>)> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  if (!on_success) {
    RTC_LOG(LS_WARNING) << "No success callback provided. Do nothing.";
    return;
  }
  if (!quic_transport_client_.get() || !quic_client_connected_) {
    if (on_failure != nullptr) {
      event_queue_->PostTask([on_failure]() {
        std::unique_ptr<Exception> e(new Exception(
            ExceptionType::kConferenceUnknown,
            "Cannot create send stream without quic server connected."));
        on_failure(std::move(e));
      });
    }
    return;
  }
  // Sychronous call for creating the stream.
  owt::quic::QuicTransportStreamInterface* quic_stream =
      quic_transport_client_->CreateOutgoingUnidirectionalStream();
  on_success(std::make_shared<owt::base::WritableStream>(quic_stream));
}

}  // namespace conference
}  // namespace owt
