/*
 * Intel License
 */
#include "webrtc/rtc_base/base64.h"
#include "webrtc/rtc_base/criticalsection.h"
#include "webrtc/rtc_base/logging.h"
#include "webrtc/rtc_base/task_queue.h"
#include "talk/ics/sdk/base/stringutils.h"
#include "talk/ics/sdk/include/cpp/ics/conference/conferenceexception.h"
#include "talk/ics/sdk/include/cpp/ics/conference/conferenceclient.h"

#include "talk/ics/sdk/include/cpp/ics/conference/conferencepublication.h"

namespace ics {
namespace conference {

ConferencePublication::ConferencePublication(std::shared_ptr<ConferenceClient> client, std::string id)
      : id_(id),
        ended_(false),
        conference_client_(client) {
  auto that = conference_client_.lock();
  if (that != nullptr)
    event_queue_ = that->event_queue_;
}

void ConferencePublication::Mute(
    TrackKind track_kind,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
   auto that = conference_client_.lock();
   if (that == nullptr || ended_) {
     std::string failure_message(
        "Session ended.");
     if (on_failure != nullptr && event_queue_.get()) {
       event_queue_->PostTask([on_failure, failure_message]() {
         std::unique_ptr<ConferenceException> e(new ConferenceException(
            ConferenceException::kUnknown, failure_message));
         on_failure(std::move(e));
       });
     }
   } else {
      that->Mute(id_, track_kind, on_success, on_failure);
   }
}

void ConferencePublication::UnMute(
    TrackKind track_kind,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
   auto that = conference_client_.lock();
   if (that == nullptr || ended_) {
     std::string failure_message(
        "Session ended.");
     if (on_failure != nullptr && event_queue_.get()) {
       event_queue_->PostTask([on_failure, failure_message]() {
         std::unique_ptr<ConferenceException> e(new ConferenceException(
            ConferenceException::kUnknown, failure_message));
         on_failure(std::move(e));
       });
     }
   } else {
      that->UnMute(id_, track_kind, on_success, on_failure);
   }
}


void ConferencePublication::GetStats(
    std::function<void(std::shared_ptr<ConnectionStats>)> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
   auto that = conference_client_.lock();
   if (that == nullptr || ended_) {
     std::string failure_message(
        "Session ended.");
     if (on_failure != nullptr && event_queue_.get()) {
       event_queue_->PostTask([on_failure, failure_message]() {
         std::unique_ptr<ConferenceException> e(new ConferenceException(
            ConferenceException::kUnknown, failure_message));
         on_failure(std::move(e));
       });
     }
   } else {
      that->GetConnectionStats(id_, on_success, on_failure);
   }
}

void ConferencePublication::Stop(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
  auto that = conference_client_.lock();
  if (that == nullptr || ended_) {
    std::string failure_message(
      "Session ended.");
    if (on_failure != nullptr && event_queue_.get()) {
      event_queue_->PostTask([on_failure, failure_message]() {
        std::unique_ptr<ConferenceException> e(new ConferenceException(
           ConferenceException::kUnknown, failure_message));
        on_failure(std::move(e));
      });
    }
  } else {
    that->UnPublish(id_, on_success, on_failure);
    ended_ = true;
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
      LOG(LS_INFO) << "Adding duplicate observer.";
      return;
  }
  observers_.push_back(observer);
}

void ConferencePublication::RemoveObserver(PublicationObserver& observer) {
  const std::lock_guard<std::mutex> lock(observer_mutex_);
  observers_.erase(std::find_if(
      observers_.begin(), observers_.end(),
      [&](std::reference_wrapper<PublicationObserver> o) -> bool {
        return &observer == &(o.get());
      }));
}

}
}
