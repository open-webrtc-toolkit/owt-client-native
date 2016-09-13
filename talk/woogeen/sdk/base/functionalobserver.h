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

  ReportType GetReportType(const webrtc::StatsReport* report) {
    //check if it's ssrc report
    bool is_sending(0);
    bool is_video(0);
    bool is_ssrc(false);
    bool is_bwe(false);
    bool is_local_candidate(false);
    bool is_remote_candidate(false);
    bool is_candidate_pair(false);
    if (report->type() == webrtc::StatsReport::kStatsReportTypeSsrc) {
      is_ssrc = true;
      if (report->FindValue(webrtc::StatsReport::kStatsValueNameBytesSent)) //this is sending
        is_sending = true;

      if (report->FindValue(webrtc::StatsReport::kStatsValueNameFrameWidthSent)
          || report->FindValue(webrtc::StatsReport::kStatsValueNameFrameWidthReceived))
        is_video = true;

    } else if (report->type() == webrtc::StatsReport::kStatsReportTypeBwe) {
      is_bwe = true;
    } else if (report->type() ==
               webrtc::StatsReport::kStatsReportTypeIceLocalCandidate) {
      is_local_candidate = true;
    } else if (report->type() ==
               webrtc::StatsReport::kStatsReportTypeIceRemoteCandidate) {
      is_remote_candidate = true;
    } else if (report->type() == webrtc::StatsReport::kStatsReportTypeCandidatePair){
      is_candidate_pair = true;
    }

    if (is_ssrc & is_sending & !is_video) {
      return REPORT_AUDIO_SENDER;
    } else if (is_ssrc & !is_sending & !is_video) {
      return REPORT_AUDIO_RECEIVER;
    } else if (is_ssrc & is_sending & is_video) {
      return REPORT_VIDEO_SENDER;
    } else if (is_ssrc & !is_sending & is_video) {
      return REPORT_VIDEO_RECEIVER;
    } else if (is_bwe) {
      return REPORT_VIDEO_BWE;
    } else if (is_local_candidate) {
      return REPORT_LOCAL_CANDIDATE;
    } else if (is_remote_candidate) {
      return REPORT_REMOTE_CANDIDATE;
    } else if (is_candidate_pair) {
      return REPORT_CANDIDATE_PAIR;
    } else {
      return REPORT_TYPE_UKNOWN;
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
