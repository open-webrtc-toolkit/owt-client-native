// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include <algorithm>
#include "webrtc/rtc_base/third_party/base64/base64.h"
#include "webrtc/rtc_base/critical_section.h"
#include "webrtc/rtc_base/logging.h"
#include "webrtc/rtc_base/task_queue.h"
#include "talk/owt/sdk/base/stringutils.h"
#include "talk/owt/sdk/include/cpp/owt/p2p/p2pclient.h"
#include "talk/owt/sdk/include/cpp/owt/p2p/p2ppublication.h"
namespace owt {
namespace p2p {
P2PPublication::P2PPublication(std::shared_ptr<P2PClient> client, std::string target_id, std::shared_ptr<LocalStream> stream)
    : target_id_(target_id),
      local_stream_(stream),
      p2p_client_(client),
      ended_(false) {
  auto that = p2p_client_.lock();
  if (that != nullptr)
    event_queue_ = that->event_queue_;
}
/// Deprecated. Get connection stats of current publication.
void P2PPublication::GetStats(
    std::function<void(std::shared_ptr<ConnectionStats>)> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  auto that = p2p_client_.lock();
  if (that == nullptr || ended_) {
    std::string failure_message(
       "Session ended.");
    if (on_failure != nullptr) {
      event_queue_->PostTask([on_failure, failure_message]() {
        std::unique_ptr<Exception> e(new Exception(
           ExceptionType::kP2PUnknown, failure_message));
        on_failure(std::move(e));
      });
    }
  } else {
     that->GetConnectionStats(target_id_, on_success, on_failure);
  }
}

/// Get connection stats of current publication.
void P2PPublication::GetStats(
    std::function<void(std::shared_ptr<RTCStatsReport>)> on_success,
    std::function<void(std::unique_ptr<Exception>)> on_failure) {
  auto that = p2p_client_.lock();
  if (that == nullptr || ended_) {
    std::string failure_message("Session ended.");
    if (on_failure != nullptr) {
      event_queue_->PostTask([on_failure, failure_message]() {
        std::unique_ptr<Exception> e(
            new Exception(ExceptionType::kP2PUnknown, failure_message));
        on_failure(std::move(e));
      });
    }
  } else {
    that->GetConnectionStats(target_id_, on_success, on_failure);
  }
}

/// Stop current publication.
void P2PPublication::Stop() {
  auto that = p2p_client_.lock();
  if (that == nullptr || ended_) {
    return;
  } else {
    that->Unpublish(target_id_, local_stream_, nullptr, nullptr);
    ended_ = true;
    const std::lock_guard<std::mutex> lock(observer_mutex_);
    for (auto its = observers_.begin(); its != observers_.end(); ++its)
      (*its).get().OnEnded();
  }
}

// Stop current publication for an unpublished stream.
void P2PPublication::StopUnpublishedStream() {
  auto that = p2p_client_.lock();
  if (that == nullptr || ended_) {
    return;
  } else {
    // Caller is responsible for unpublishing the stream before calling this method.
    // Caller should do the equivalent of the following:
    // that->Unpublish(target_id_, local_stream_, nullptr, nullptr);
    ended_ = true;
    const std::lock_guard<std::mutex> lock(observer_mutex_);
    for (auto its = observers_.begin(); its != observers_.end(); ++its)
      (*its).get().OnEnded();
  }
}

void P2PPublication::AddObserver(PublicationObserver& observer) {
  const std::lock_guard<std::mutex> lock(observer_mutex_);
  std::vector<std::reference_wrapper<PublicationObserver>>::iterator it =
      std::find_if(
          observers_.begin(), observers_.end(),
          [&](std::reference_wrapper<PublicationObserver> o) -> bool {
            return &observer == &(o.get());
  });
  if (it != observers_.end()) {
    RTC_LOG(LS_WARNING) << "Adding duplicate observer.";
    return;
  }
  observers_.push_back(observer);
}
void P2PPublication::RemoveObserver(PublicationObserver& observer) {
  const std::lock_guard<std::mutex> lock(observer_mutex_);
  auto it = std::find_if(
    observers_.begin(), observers_.end(),
    [&](std::reference_wrapper<PublicationObserver> o) -> bool {
      return &observer == &(o.get());
  });
  if (it == observers_.end()) {
    RTC_LOG(LS_WARNING) << "Trying to delete non-existing observer.";
    return;
  }
  observers_.erase(it);
}
}
}
