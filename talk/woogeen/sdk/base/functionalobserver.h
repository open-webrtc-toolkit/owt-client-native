/*
 * Intel License
 */

#ifndef WOOGEEN_BASE_FUNCTIONALOBSERVER_H_
#define WOOGEEN_BASE_FUNCTIONALOBSERVER_H_

#include <functional>
#include "talk/app/webrtc/jsep.h"
#include "webrtc/base/scoped_ref_ptr.h"

namespace woogeen {
namespace base {
// A webrtc::CreateSessionDescriptionObserver implementation used to invoke user
// defined function when creating description complete.
class FunctionalCreateSessionDescriptionObserver
    : public webrtc::CreateSessionDescriptionObserver {
 public:
  static rtc::scoped_refptr<FunctionalCreateSessionDescriptionObserver> Create(
      std::function<void(webrtc::SessionDescriptionInterface*)> on_success,
      std::function<void(const std::string&)> on_failure);
  virtual void OnSuccess(webrtc::SessionDescriptionInterface* desc);
  virtual void OnFailure(const std::string& error);

 protected:
  FunctionalCreateSessionDescriptionObserver(
      std::function<void(webrtc::SessionDescriptionInterface*)> on_success,
      std::function<void(const std::string&)> on_failure);

 private:
  std::function<void(webrtc::SessionDescriptionInterface*)> on_success_;
  std::function<void(const std::string&)> on_failure_;
};

// A webrtc::SetSessionDescriptionObserver implementation used to invoke user
// defined function when set description complete.
class FunctionalSetSessionDescriptionObserver
    : public webrtc::SetSessionDescriptionObserver {
 public:
  static rtc::scoped_refptr<FunctionalSetSessionDescriptionObserver> Create(
      std::function<void()> on_success,
      std::function<void(const std::string&)> on_failure);
  virtual void OnSuccess();
  virtual void OnFailure(const std::string& error);

 protected:
  FunctionalSetSessionDescriptionObserver(
      std::function<void()> on_success,
      std::function<void(const std::string& error)> on_failure);

 private:
  std::function<void()> on_success_;
  std::function<void(const std::string& error)> on_failure_;
};
}
}

#endif  // WOOGEEN_BASE_FUNCTIONALOBSERVER_H_
