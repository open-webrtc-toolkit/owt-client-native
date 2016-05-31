/*
 * Intel License
 */

#ifndef WOOGEEN_BASE_FUNCTIONALOBSERVER_H_
#define WOOGEEN_BASE_FUNCTIONALOBSERVER_H_

#include <functional>

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
    REPORT_TYPE_UKNOWN = 99,
  };

  std::function<void(std::shared_ptr<ConnectionStats>)> on_complete_;

  ReportType GetReportType(const webrtc::StatsReport* report) {
    //check if it's ssrc report
    bool isSending = 0;
    bool isVideo = 0;
    bool isSSRC = false;
    bool isBWE = false;
    if (report->type() == webrtc::StatsReport::kStatsReportTypeSsrc) {
      isSSRC = true;
      if (report->FindValue(webrtc::StatsReport::kStatsValueNameBytesSent)) //this is sending
        isSending = true;

      if (report->FindValue(webrtc::StatsReport::kStatsValueNameFrameWidthSent)
          || report->FindValue(webrtc::StatsReport::kStatsValueNameFrameWidthReceived))
        isVideo = true;

    } else if(report->type() == webrtc::StatsReport::kStatsReportTypeBwe){
      isBWE = true;
    }

    if (isSSRC & isSending & !isVideo) {
      return REPORT_AUDIO_SENDER;
    } else if (isSSRC & !isSending & !isVideo) {
      return REPORT_AUDIO_RECEIVER;
    } else if (isSSRC & isSending & isVideo) {
      return REPORT_VIDEO_SENDER;
    } else if (isSSRC & !isSending & isVideo) {
      return REPORT_VIDEO_RECEIVER;
    } else if (isBWE) {
      return REPORT_VIDEO_BWE;
    } else
      return REPORT_TYPE_UKNOWN;
  }
};
}
}

#endif  // WOOGEEN_BASE_FUNCTIONALOBSERVER_H_
