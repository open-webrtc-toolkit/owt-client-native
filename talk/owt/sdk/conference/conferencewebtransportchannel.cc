// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "talk/owt/sdk/conference/conferencewebtransportchannel.h"
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include "talk/owt/sdk/base/functionalobserver.h"
#include "talk/owt/sdk/base/mediautils.h"
#include "talk/owt/sdk/include/cpp/owt/base/stream.h"
#include "talk/owt/sdk/include/cpp/owt/base/globalconfiguration.h"
#include "webrtc/rtc_base/checks.h"
#include "webrtc/rtc_base/logging.h"
#include "owt/quic/quic_definitions.h"
#include "owt/quic/quic_transport_client_interface.h"
#include "owt/quic/quic_transport_factory.h"
#include "owt/quic/quic_transport_stream_interface.h"

using namespace rtc;
using namespace owt::quic;

namespace owt {
namespace conference {
using std::string;

// Stream option member key
const std::string kStreamOptionStreamIdKey = "streamId";
const std::string kStreamOptionStateKey = "state";
const std::string kStreamOptionDataKey = "type";
const std::string kStreamOptionWebTransportKey = "data";
const std::string kStreamOptionAttributesKey = "attributes";

ConferenceWebTransportChannel::ConferenceWebTransportChannel(
    const std::string& url,
    const std::string& webtransport_id,
    const std::string& webtransport_token,
    std::shared_ptr<ConferenceSocketSignalingChannel> signaling_channel,
    std::shared_ptr<rtc::TaskQueue> event_queue)
    : signaling_channel_(signaling_channel),
      connected_(false),
      event_queue_(event_queue),
      url_(url),
      transport_id_(webtransport_id),
      webtransport_token_(webtransport_token) {
  quic_transport_factory_.reset(owt::quic::QuicTransportFactory::Create());
  quic_client_connected_ = false;
  RTC_CHECK(signaling_channel_);
}

ConferenceWebTransportChannel::~ConferenceWebTransportChannel() {
  RTC_LOG(LS_INFO) << "Deconstruct conference WebTransport channel";
  // We will rely on conference client to stop all publications/subscriptions.
}

void ConferenceWebTransportChannel::Authenticate() {
  // Send the transportTicket acquired on joining.
  if (!auth_stream_)
    return;
  // 128-bit of zero indicates this is a stream for signaling.
  uint8_t signaling_stream_id[16] = {0};
  auth_stream_->Write(&signaling_stream_id[0], 16);
  // Send token as described in
  // https://github.com/open-webrtc-toolkit/owt-server/blob/20e8aad216cc446095f63c409339c34c7ee770ee/doc/design/quic-transport-payload-format.md.
  uint32_t token_size = webtransport_token_.length();
  memcpy(&signaling_stream_id[0], &token_size, sizeof(uint32_t));
  auth_stream_->Write(&signaling_stream_id[0], sizeof(uint32_t));
  auth_stream_->Write((uint8_t*)(webtransport_token_.c_str()), token_size);
}

void ConferenceWebTransportChannel::Connect() {
  std::vector<owt::base::cert_fingerprint_t> trusted_fingerprints =
      GlobalConfiguration::GetTrustedQuicCertificateFingerPrints();
  size_t cert_fingerprint_size = trusted_fingerprints.size();

  owt::quic::QuicTransportClientInterface::Parameters quic_params;
  memset(&quic_params, 0, sizeof(quic_params));
  owt::quic::CertificateFingerprint* fingerprint =
      new owt::quic::CertificateFingerprint[cert_fingerprint_size];
  if (cert_fingerprint_size > 0) {
    quic_params.server_certificate_fingerprints = &fingerprint;
    quic_params.server_certificate_fingerprints_length = cert_fingerprint_size;
  }
  int fingerprint_idx = 0;
  for (auto fingerprint : trusted_fingerprints) {
    memcpy(quic_params.server_certificate_fingerprints[fingerprint_idx],
           fingerprint.data(), fingerprint.size());
  }
  RTC_LOG(LS_ERROR) << "Creating Quic transport client.";
  quic_transport_client_.reset(quic_transport_factory_->CreateQuicTransportClient(
      url_.c_str(), quic_params));
  if (quic_transport_client_.get()) {
    quic_transport_client_->SetVisitor(this);
    // Async. Connect status will be notified through visitor.
    quic_transport_client_->Connect();
  } else {
    RTC_LOG(LS_ERROR) << "Failed to create Quic transport client.";
  }
}

void ConferenceWebTransportChannel::AddObserver(
    ConferenceWebTransportChannelObserver* observer) {
  RTC_CHECK(observer);
  observer_ = observer;
}
void ConferenceWebTransportChannel::RemoveObserver() {
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
    std::shared_ptr<owt::base::LocalStream> stream,
    std::function<void(std::string, std::string)> on_success,
    std::function<void(std::unique_ptr<owt::base::Exception>)> on_failure) {
  RTC_LOG(LS_INFO) << "Publish a local stream.";
  if (!CheckNullPointer((uintptr_t)stream.get(), on_failure)) {
    RTC_LOG(LS_INFO) << "Local stream cannot be nullptr.";
  }
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
  transport_ptr->get_map()["id"] = sio::string_message::create(transport_id_);
  SendPublishMessage(options, stream, on_success, on_failure);
}


void ConferenceWebTransportChannel::Subscribe(
    std::shared_ptr<owt::base::RemoteStream> stream,
    std::function<void(std::string)> on_success,
    std::function<void(std::unique_ptr<owt::base::Exception>)> on_failure) {
  if (!CheckNullPointer((uintptr_t)stream.get(), on_failure)) {
    RTC_LOG(LS_ERROR) << "Remote stream cannot be nullptr.";
    return;
  }
  if (!stream->DataEnabled()) {
    if (on_failure) {
      event_queue_->PostTask([on_failure]() {
        std::unique_ptr<Exception> e(new Exception(
            ExceptionType::kConferenceUnknown, "Non-data stream not supported by WebTransport."));
        on_failure(std::move(e));
      });
    }
  }

  sio::message::ptr sio_options = sio::object_message::create();
  sio::message::ptr media_options = sio::object_message::create();
  sio_options->get_map()["media"] = media_options;
  sio::message::ptr transport_options = sio::object_message::create();
  transport_options->get_map()["type"] = sio::string_message::create("quic");
  transport_options->get_map()["id"] =
      sio::string_message::create(transport_id_);
  sio::message::ptr data_options = sio::object_message::create();
  data_options->get_map()["from"] =
      sio::string_message::create(stream->Origin());
  signaling_channel_->SendInitializationMessage(
      sio_options, "", "data-stream",
      [this, on_success, on_failure](std::string session_id,
                                     std::string transport_id) {
        if (on_success) {
          on_success(session_id);
        }
      },
      on_failure);  // TODO: on_failure
}

void ConferenceWebTransportChannel::Unpublish(
    const std::string& session_id,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<owt::base::Exception>)> on_failure) {
  const std::lock_guard<std::mutex> lock(published_session_ids_mutex_);
  auto itr = std::find(published_session_ids_.begin(),
                       published_session_ids_.end(), session_id);
  if (itr == published_session_ids_.end()) {
    RTC_LOG(LS_ERROR) << "No publication matches session id:" << session_id;
    if (on_failure != nullptr) {
      event_queue_->PostTask([on_failure]() {
        std::unique_ptr<owt::base::Exception> e(new owt::base::Exception(
            owt::base::ExceptionType::kConferenceUnknown,
            "Invalid session to be unpublishd."));
        on_failure(std::move(e));
      });
    }
    return;
  }
  signaling_channel_->SendStreamEvent("unpublish", session_id,
                                      RunInEventQueue(on_success), on_failure);
  published_session_ids_.erase(itr);
}

void ConferenceWebTransportChannel::Unsubscribe(
    const std::string& session_id,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<owt::base::Exception>)> on_failure) {
  std::lock_guard<std::mutex> lock(subscribed_session_ids_mutex_);
  auto itr = std::find(subscribed_session_ids_.begin(),
                       subscribed_session_ids_.end(), session_id);
  if (itr == subscribed_session_ids_.end()) {
    RTC_LOG(LS_ERROR) << "No subscription matches session id:" << session_id;
    if (on_failure != nullptr) {
      event_queue_->PostTask([on_failure]() {
        std::unique_ptr<owt::base::Exception> e(
            new owt::base::Exception(owt::base::ExceptionType::kConferenceUnknown,
                          "Invalid session to be unsubscribed."));
        on_failure(std::move(e));
      });
    }
    return;
  }
  signaling_channel_->SendStreamEvent("unsubscribe", session_id,
                                      RunInEventQueue(on_success), on_failure);
  subscribed_session_ids_.erase(itr);
}

void ConferenceWebTransportChannel::SendStreamControlMessage(
    const std::string& session_id,
    const std::string& in_action,
    const std::string& out_action,
    const std::string& operation,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) const {
  std::string action = "";
  // Check if this session is pub or sub.
  auto itr_pub = std::find(published_session_ids_.begin(),
                       published_session_ids_.end(), session_id);
  auto itr_sub = std::find(subscribed_session_ids_.begin(),
                           subscribed_session_ids_.end(), session_id);
  if (itr_pub != published_session_ids_.end() && out_action != "") {
    action = out_action;
    signaling_channel_->SendStreamControlMessage(session_id, action, operation,
                                                 on_success, on_failure);
  } else if (itr_sub != subscribed_session_ids_.end() && in_action != "") {
    action = in_action;
    signaling_channel_->SendSubscriptionControlMessage(
        session_id, action, operation, on_success, on_failure);
  }
}

void ConferenceWebTransportChannel::SendPublishMessage(
    sio::message::ptr options,
    std::shared_ptr<owt::base::LocalStream> stream,
    std::function <void(std::string session_id, std::string transport_id)> on_success,
    std::function<void(std::unique_ptr<owt::base::Exception>)> on_failure) {
  std::weak_ptr<owt::conference::ConferenceWebTransportChannel> weak_this = shared_from_this();
  signaling_channel_->SendInitializationMessage(
      options, "data-stream", "",
      [stream, weak_this, on_success, on_failure](std::string session_id, std::string transport_id) {
        auto that = weak_this.lock();
        if (that) {
          if (on_success) {
            on_success(session_id, transport_id);
          }
        }
      },
      on_failure);
}

void ConferenceWebTransportChannel::OnStreamError(
    const std::string& error_message) {
  // Not implemented.
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

void ConferenceWebTransportChannel::OnConnected() {
  RTC_LOG(LS_ERROR) << "Quic client connected.";
  //Authenticate the client.
  RTC_DCHECK(quic_transport_client_.get());
  auth_stream_ = quic_transport_client_->CreateBidirectionalStream();
  auth_stream_observer_ = std::make_unique<owt::conference::ConferenceWebTransportChannel::AuthStreamObserver>(this);
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

// This gets called for subscription case. After sucessful
// subscribe signaling, MCU will create the send stream on
// server side, and this results in client's OnIncomingStream
// to be invoked. MCU will further send the session id on the
// incoming stream. This will be used to associate the stream
// with subscription.
void ConferenceWebTransportChannel::OnIncomingStream(
    owt::quic::QuicTransportStreamInterface* stream) {
  // At the time of this callback, the session id is still unknown.
  // We will need to wait until we read it over signaling session.
  RTC_LOG(LS_INFO) << "Incoming quic stream.";
  owt::conference::ConferenceWebTransportChannel::IncomingStreamObserver* stream_observer =
      new owt::conference::ConferenceWebTransportChannel::IncomingStreamObserver(this, stream);
  stream->SetVisitor(stream_observer);
}

void ConferenceWebTransportChannel::OnStreamSessionId(const std::string& session_id,
    owt::quic::QuicTransportStreamInterface* stream) {
  // TODO: remove the stream observer for this stream first.
  if (observer_) {
    observer_->OnIncomingStream(session_id, stream);
  }
}

void ConferenceWebTransportChannel::CreateSendStream(
    std::function<void(std::shared_ptr<owt::base::LocalStream>)> on_success,
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
      quic_transport_client_->CreateBidirectionalStream();
  // For local stream session id is not specified at stream creation time.
  std::shared_ptr<owt::base::QuicStream> writable_stream =
      std::make_shared<owt::base::QuicStream>(quic_stream, "0");
  int error_code = 0;
  on_success(owt::base::LocalStream::Create(writable_stream, error_code));
}

}  // namespace conference
}  // namespace owt
