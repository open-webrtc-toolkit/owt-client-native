/*
 * Intel License
 */

#include "talk/woogeen/sdk/base/functionalobserver.h"

namespace woogeen {
namespace base {

FunctionalCreateSessionDescriptionObserver::
    FunctionalCreateSessionDescriptionObserver(
        std::function<void(webrtc::SessionDescriptionInterface*)> on_success,
        std::function<void(const std::string&)> on_failure)
    : on_success_(on_success), on_failure_(on_failure) {}

rtc::scoped_refptr<FunctionalCreateSessionDescriptionObserver>
FunctionalCreateSessionDescriptionObserver::Create(
    std::function<void(webrtc::SessionDescriptionInterface*)> on_success,
    std::function<void(const std::string&)> on_failure) {
  return new rtc::RefCountedObject<FunctionalCreateSessionDescriptionObserver>(
      on_success, on_failure);
}

void FunctionalCreateSessionDescriptionObserver::OnSuccess(
    webrtc::SessionDescriptionInterface* desc) {
  if (on_success_ != nullptr) {
    on_success_(desc);
  }
}

void FunctionalCreateSessionDescriptionObserver::OnFailure(
    const std::string& error) {
  if (on_failure_ != nullptr) {
    on_failure_(error);
  }
}

FunctionalSetSessionDescriptionObserver::
    FunctionalSetSessionDescriptionObserver(
        std::function<void()> on_success,
        std::function<void(const std::string&)> on_failure)
    : on_success_(on_success), on_failure_(on_failure) {}

rtc::scoped_refptr<FunctionalSetSessionDescriptionObserver>
FunctionalSetSessionDescriptionObserver::Create(
    std::function<void()> on_success,
    std::function<void(const std::string&)> on_failure) {
  return new rtc::RefCountedObject<FunctionalSetSessionDescriptionObserver>(
      on_success, on_failure);
}

void FunctionalSetSessionDescriptionObserver::OnSuccess() {
  if (on_success_ != nullptr) {
    on_success_();
  }
}

void FunctionalSetSessionDescriptionObserver::OnFailure(
    const std::string& error) {
  if (on_failure_ != nullptr) {
    on_failure_(error);
  }
}
}
}
