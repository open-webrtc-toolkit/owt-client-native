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

FunctionalStatsObserver::FunctionalStatsObserver(
    std::function<void(std::shared_ptr<ConnectionStats>)> on_complete)
    : on_complete_(on_complete) {}

rtc::scoped_refptr<FunctionalStatsObserver> FunctionalStatsObserver::Create(
    std::function<void(std::shared_ptr<ConnectionStats>)> on_complete) {
  return new rtc::RefCountedObject<FunctionalStatsObserver>(on_complete);
}

void FunctionalStatsObserver::OnComplete(const webrtc::StatsReports& reports) {
  if (on_complete_ != nullptr) {
    std::shared_ptr<ConnectionStats> connection_stats(new ConnectionStats());
    int32_t adapt_reason = static_cast<int32_t>(VideoSenderReport::AdaptReason::kUnknown);
    for (const auto* report : reports) {
      ReportType report_type = GetReportType(report);
      switch (report_type) {
      case REPORT_AUDIO_RECEIVER:
      {
        std::unique_ptr<AudioReceiverReport> audio_recv_report_ptr(new AudioReceiverReport(
            report->FindValue(webrtc::StatsReport::kStatsValueNameBytesReceived)->int64_val(),
            report->FindValue(webrtc::StatsReport::kStatsValueNamePacketsReceived)->int_val(),
            report->FindValue(webrtc::StatsReport::kStatsValueNamePacketsLost)->int_val(),
            report->FindValue(webrtc::StatsReport::kStatsValueNameCurrentDelayMs)->int_val(),
            report->FindValue(webrtc::StatsReport::kStatsValueNameCodecName)->string_val()));
         connection_stats->audio_receiver_reports.push_back(std::move(audio_recv_report_ptr));
        break;
      }
      case REPORT_AUDIO_SENDER:
      {
        std::unique_ptr<AudioSenderReport> audio_send_report_ptr(new AudioSenderReport(
            report->FindValue(webrtc::StatsReport::kStatsValueNameBytesSent)->int64_val(),
            report->FindValue(webrtc::StatsReport::kStatsValueNamePacketsSent)->int_val(),
            report->FindValue(webrtc::StatsReport::kStatsValueNamePacketsLost)->int_val(),
            report->FindValue(webrtc::StatsReport::kStatsValueNameRtt)->int64_val(),
            report->FindValue(webrtc::StatsReport::kStatsValueNameCodecName)->string_val()));
        connection_stats->audio_sender_reports.push_back(std::move(audio_send_report_ptr));
        break;
      }
      case  REPORT_VIDEO_RECEIVER:
      {
        std::unique_ptr<VideoReceiverReport> video_recv_report_ptr(new VideoReceiverReport(
            report->FindValue(webrtc::StatsReport::kStatsValueNameBytesReceived)->int64_val(),
            report->FindValue(webrtc::StatsReport::kStatsValueNamePacketsReceived)->int_val(),
            report->FindValue(webrtc::StatsReport::kStatsValueNamePacketsLost)->int_val(),
            report->FindValue(webrtc::StatsReport::kStatsValueNameFirsSent)->int_val(),
            report->FindValue(webrtc::StatsReport::kStatsValueNamePlisSent)->int_val(),
            report->FindValue(webrtc::StatsReport::kStatsValueNameNacksSent)->int_val(),
            report->FindValue(webrtc::StatsReport::kStatsValueNameFrameHeightReceived)->int_val(),
            report->FindValue(webrtc::StatsReport::kStatsValueNameFrameWidthReceived)->int_val(),
            report->FindValue(webrtc::StatsReport::kStatsValueNameFrameRateReceived)->int_val(),
            report->FindValue(webrtc::StatsReport::kStatsValueNameFrameRateOutput)->int_val(),
            report->FindValue(webrtc::StatsReport::kStatsValueNameCurrentDelayMs)->int_val(),
            report->FindValue(webrtc::StatsReport::kStatsValueNameCodecName)->string_val()));
        connection_stats->video_receiver_reports.push_back(std::move(video_recv_report_ptr));
        break;
      }
      case REPORT_VIDEO_SENDER:
      {
        adapt_reason = static_cast<int32_t>(VideoSenderReport::AdaptReason::kUnknown);
        if (report->FindValue(webrtc::StatsReport::kStatsValueNameCpuLimitedResolution)->bool_val())
          adapt_reason = static_cast<int32_t>(VideoSenderReport::AdaptReason::kCpuLimitation);
        else if (report->FindValue(webrtc::StatsReport::kStatsValueNameBandwidthLimitedResolution)->bool_val())
          adapt_reason = static_cast<int32_t>(VideoSenderReport::AdaptReason::kBandwidthLimitation);
        else if (report->FindValue(webrtc::StatsReport::kStatsValueNameViewLimitedResolution)->bool_val())
          adapt_reason = static_cast<int32_t>(VideoSenderReport::AdaptReason::kViewLimitation);
        std::unique_ptr<VideoSenderReport> video_send_report_ptr(new VideoSenderReport(
            report->FindValue(webrtc::StatsReport::kStatsValueNameBytesSent)->int64_val(),
            report->FindValue(webrtc::StatsReport::kStatsValueNamePacketsSent)->int_val(),
            report->FindValue(webrtc::StatsReport::kStatsValueNamePacketsLost)->int_val(),
            report->FindValue(webrtc::StatsReport::kStatsValueNameFirsReceived)->int_val(),
            report->FindValue(webrtc::StatsReport::kStatsValueNamePlisReceived)->int_val(),
            report->FindValue(webrtc::StatsReport::kStatsValueNameNacksReceived)->int_val(),
            report->FindValue(webrtc::StatsReport::kStatsValueNameFrameHeightSent)->int_val(),
            report->FindValue(webrtc::StatsReport::kStatsValueNameFrameWidthSent)->int_val(),
            report->FindValue(webrtc::StatsReport::kStatsValueNameFrameRateSent)->int_val(),
            adapt_reason,
            report->FindValue(webrtc::StatsReport::kStatsValueNameAdaptationChanges)->int_val(),
            report->FindValue(webrtc::StatsReport::kStatsValueNameRtt)->int64_val(),
            report->FindValue(webrtc::StatsReport::kStatsValueNameCodecName)->string_val()));
        connection_stats->video_sender_reports.push_back(std::move(video_send_report_ptr));
        break;
      }
      case REPORT_VIDEO_BWE:
      {
        connection_stats->video_bandwidth_stats.available_send_bandwidth =
            report->FindValue(webrtc::StatsReport::kStatsValueNameAvailableSendBandwidth)->int_val();
        connection_stats->video_bandwidth_stats.available_receive_bandwidth =
            report->FindValue(webrtc::StatsReport::kStatsValueNameAvailableReceiveBandwidth)->int_val();
        connection_stats->video_bandwidth_stats.transmit_bitrate =
            report->FindValue(webrtc::StatsReport::kStatsValueNameTransmitBitrate)->int_val();
        connection_stats->video_bandwidth_stats.retransmit_bitrate =
            report->FindValue(webrtc::StatsReport::kStatsValueNameRetransmitBitrate)->int_val();
        break;
      }
      default:
        break;
      }
    }

    on_complete_(connection_stats);
  }
}
}
}
