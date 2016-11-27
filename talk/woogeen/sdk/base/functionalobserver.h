/*
 * Intel License
 */

#ifndef WOOGEEN_BASE_FUNCTIONALOBSERVER_H_
#define WOOGEEN_BASE_FUNCTIONALOBSERVER_H_

#include <functional>
#include <unordered_map>

#include "webrtc/api/jsep.h"
#include "webrtc/api/peerconnectioninterface.h"
#include "webrtc/base/scoped_ref_ptr.h"
#include "talk/woogeen/sdk/include/cpp/woogeen/base/connectionstats.h"

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

// A StatsObserver implementation used to invoke user defined function to
// retrieve current statistics data.
class FunctionalStatsObserver : public webrtc::StatsObserver {
 public:
  static rtc::scoped_refptr<FunctionalStatsObserver> Create(
      std::function<void(std::shared_ptr<ConnectionStats>)> on_complete);

  virtual void OnComplete(const webrtc::StatsReports& reports);

 protected:
  FunctionalStatsObserver(
      std::function<void(std::shared_ptr<ConnectionStats>)> on_complete);

 private:
  enum ReportType {
    REPORT_AUDIO_SENDER = 1,
    REPORT_AUDIO_RECEIVER,
    REPORT_VIDEO_SENDER,
    REPORT_VIDEO_RECEIVER,
    REPORT_VIDEO_BWE,
    REPORT_LOCAL_CANDIDATE,
    REPORT_REMOTE_CANDIDATE,
    REPORT_CANDIDATE_PAIR,
    REPORT_TYPE_UKNOWN = 99,
  };

  std::function<void(std::shared_ptr<ConnectionStats>)> on_complete_;

  ReportType GetReportType(const webrtc::StatsReport* report);
  template <class T>
  T GetValue(std::function<T(const webrtc::StatsReport::Value&)> get_value,
             const webrtc::StatsReport* report,
             const webrtc::StatsReport::StatsValueName name,
             T default_value) {
    auto item = report->FindValue(name);
    if (item) {
      return get_value(*item);
    } else {
      return default_value;
    }
  }

  IceCandidateType GetCandidateType(const std::string& type);
  TransportProtocolType GetTransportProtocolType(const std::string& protocol);
  std::shared_ptr<IceCandidateReport> GetIceCandidateReport(const webrtc::StatsReport& report);

  std::unordered_map<std::string, std::shared_ptr<IceCandidateReport>> local_candidate_map_;
  std::unordered_map<std::string, std::shared_ptr<IceCandidateReport>> remote_candidate_map_;
};
}
}

#endif  // WOOGEEN_BASE_FUNCTIONALOBSERVER_H_
