// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include <algorithm>
#include "webrtc/rtc_base/third_party/base64/base64.h"
#include "webrtc/rtc_base/logging.h"
#include "webrtc/rtc_base/task_queue.h"
#include "talk/owt/sdk/base/stringutils.h"
#include "talk/owt/sdk/include/cpp/owt/base/exception.h"
#include "talk/owt/sdk/include/cpp/owt/conference/conferenceclient.h"
#include "talk/owt/sdk/include/cpp/owt/conference/conferencepublication.h"
using namespace rtc;
namespace owt {
namespace conference {
ConferencePublication::ConferencePublication(std::shared_ptr<ConferenceClient> client, const std::string& pub_id,
                                             const std::string& stream_id)
      : id_(pub_id),
        stream_id_(stream_id),
        ended_(false),
        conference_client_(client) {
  auto that = conference_client_.lock();
  if (that != nullptr)
    event_queue_ = that->event_queue_;
}
ConferencePublication::~ConferencePublication() {
  // All the streamupdate observers must be removed on conference client
  // before publication is destructed.
  auto that = conference_client_.lock();
  if (that != nullptr)
    that->RemoveStreamUpdateObserver(*this);
}
void ConferencePublication::Mute(
    TrackKind track_kind,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
   auto that = conference_client_.lock();
   if (that == nullptr || ended_) {
     std::string failure_message(
        "Session ended.");
     if (on_failure != nullptr && event_queue_.get()) {
       event_queue_->PostTask([on_failure, failure_message]() {
         std::unique_ptr<Exception> e(new Exception(
            ExceptionType::kConferenceUnknown, failure_message));
         on_failure(std::move(e));
       });
     }
   } else {
      that->Mute(id_, track_kind, on_success, on_failure);
   }
}
void ConferencePublication::Unmute(
    TrackKind track_kind,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
   auto that = conference_client_.lock();
   if (that == nullptr || ended_) {
     std::string failure_message(
        "Session ended.");
     if (on_failure != nullptr && event_queue_.get()) {
       event_queue_->PostTask([on_failure, failure_message]() {
         std::unique_ptr<Exception> e(new Exception(
            ExceptionType::kConferenceUnknown, failure_message));
         on_failure(std::move(e));
       });
     }
   } else {
      that->Unmute(id_, track_kind, on_success, on_failure);
   }
}
void ConferencePublication::GetStats(
    std::function<void(std::shared_ptr<ConnectionStats>)> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
   auto that = conference_client_.lock();
   if (that == nullptr || ended_) {
     std::string failure_message(
        "Session ended.");
     if (on_failure != nullptr && event_queue_.get()) {
       event_queue_->PostTask([on_failure, failure_message]() {
         std::unique_ptr<Exception> e(new Exception(
            ExceptionType::kConferenceUnknown, failure_message));
         on_failure(std::move(e));
       });
     }
   } else {
      that->GetConnectionStats(id_, on_success, on_failure);
   }
}

void ConferencePublication::GetStats(
    std::function<void(std::shared_ptr<RTCStatsReport>)> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  auto that = conference_client_.lock();
  if (that == nullptr || ended_) {
    std::string failure_message("Session ended.");
    if (on_failure != nullptr && event_queue_.get()) {
      event_queue_->PostTask([on_failure, failure_message]() {
        std::unique_ptr<Exception> e(
            new Exception(ExceptionType::kConferenceUnknown, failure_message));
        on_failure(std::move(e));
      });
    }
  } else {
    that->GetConnectionStats(id_, on_success, on_failure);
  }
}

void ConferencePublication::GetNativeStats(
    std::function<void(const std::vector<const webrtc::StatsReport*>& reports)>
        on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
     auto that = conference_client_.lock();
   if (that == nullptr || ended_) {
     std::string failure_message(
        "Session ended.");
     if (on_failure != nullptr && event_queue_.get()) {
       event_queue_->PostTask([on_failure, failure_message]() {
         std::unique_ptr<Exception> e(new Exception(
            ExceptionType::kConferenceInvalidParam, failure_message));
         on_failure(std::move(e));
       });
     }
   } else {
     that->GetStats(id_, on_success, on_failure);
   }
}
void ConferencePublication::Stop() {
  auto that = conference_client_.lock();
  if (that == nullptr || ended_) {
    return;
  } else {
    that->UnPublish(id_, nullptr, nullptr);
    ended_ = true;
    const std::lock_guard<std::mutex> lock(observer_mutex_);
    for (auto its = observers_.begin(); its != observers_.end(); ++its) {
      (*its).get().OnEnded();
    }
  }
}
void ConferencePublication::OnStreamMuteOrUnmute(const std::string& stream_id,
                                                 TrackKind track_kind,
                                                 bool muted) {
  if (ended_ || stream_id != id_)
    return;
  for (auto its = observers_.begin(); its != observers_.end(); ++its) {
    if (muted) {
      (*its).get().OnMute(track_kind);
    } else {
      (*its).get().OnUnmute(track_kind);
    }
  }
}

void ConferencePublication::OnStreamRemoved(const std::string& stream_id) {
  if (ended_ || stream_id != id_)
    return;
  Stop();
}

void ConferencePublication::OnStreamError(const std::string& error_msg) {
  for (auto its = observers_.begin(); its != observers_.end(); ++its) {
    std::unique_ptr<Exception> e(new Exception(
        ExceptionType::kConferenceUnknown, error_msg));
    (*its).get().OnError(std::move(e));
  }
}

void ConferencePublication::AddObserver(PublicationObserver& observer) {
  const std::lock_guard<std::mutex> lock(observer_mutex_);
  std::vector<std::reference_wrapper<PublicationObserver>>::iterator it =
      std::find_if(
          observers_.begin(), observers_.end(),
          [&](std::reference_wrapper<PublicationObserver> o) -> bool {
      return &observer == &(o.get());
  });
  if (it != observers_.end()) {
      RTC_LOG(LS_INFO) << "Adding duplicate observer.";
      return;
  }
  observers_.push_back(observer);
  // Only when observer is added will we register stream update observer on conference client.
  auto that = conference_client_.lock();
  if (that != nullptr && !ended_) {
    that->AddStreamUpdateObserver(*this);
  }
}
void ConferencePublication::RemoveObserver(PublicationObserver& observer) {
  const std::lock_guard<std::mutex> lock(observer_mutex_);
  auto it = std::find_if(
    observers_.begin(), observers_.end(),
    [&](std::reference_wrapper<PublicationObserver> o) -> bool {
    return &observer == &(o.get());
  });
  // We will keep the stream update observer attached upon empty publication observer list.
  if (it != observers_.end())
    observers_.erase(it);
}
}
}
