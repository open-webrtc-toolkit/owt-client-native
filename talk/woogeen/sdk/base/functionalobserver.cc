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
        std::unique_ptr<AudioReceiverReport> audio_recv_report_ptr(
            new AudioReceiverReport(
                GetValue<int64_t>(
                    &webrtc::StatsReport::Value::int64_val, report,
                    webrtc::StatsReport::kStatsValueNameBytesReceived, -1),
                GetValue<int>(
                    &webrtc::StatsReport::Value::int_val, report,
                    webrtc::StatsReport::kStatsValueNamePacketsReceived, -1),
                GetValue<int>(&webrtc::StatsReport::Value::int_val, report,
                              webrtc::StatsReport::kStatsValueNamePacketsLost,
                              -1),
                GetValue<int>(
                    &webrtc::StatsReport::Value::int_val, report,
                    webrtc::StatsReport::kStatsValueNameCurrentDelayMs, -1),
                GetValue<std::string>(
                    &webrtc::StatsReport::Value::string_val, report,
                    webrtc::StatsReport::kStatsValueNameCodecName, "")));
        connection_stats->audio_receiver_reports.push_back(
            std::move(audio_recv_report_ptr));
        break;
      }
      case REPORT_AUDIO_SENDER:
      {
        std::unique_ptr<AudioSenderReport> audio_send_report_ptr(
            new AudioSenderReport(
                GetValue<int64_t>(
                    &webrtc::StatsReport::Value::int64_val, report,
                    webrtc::StatsReport::kStatsValueNameBytesSent, -1),
                GetValue<int>(&webrtc::StatsReport::Value::int_val, report,
                              webrtc::StatsReport::kStatsValueNamePacketsSent,
                              -1),
                GetValue<int>(&webrtc::StatsReport::Value::int_val, report,
                              webrtc::StatsReport::kStatsValueNamePacketsLost,
                              -1),
                GetValue<int64_t>(&webrtc::StatsReport::Value::int64_val,
                                  report,
                                  webrtc::StatsReport::kStatsValueNameRtt, -1),
                report->FindValue(webrtc::StatsReport::kStatsValueNameCodecName)
                    ->string_val()));
        connection_stats->audio_sender_reports.push_back(
            std::move(audio_send_report_ptr));
        break;
      }
      case  REPORT_VIDEO_RECEIVER:
      {
        std::unique_ptr<VideoReceiverReport> video_recv_report_ptr(
            new VideoReceiverReport(
                GetValue<int64_t>(
                    &webrtc::StatsReport::Value::int64_val, report,
                    webrtc::StatsReport::kStatsValueNameBytesReceived, -1),
                GetValue<int>(
                    &webrtc::StatsReport::Value::int_val, report,
                    webrtc::StatsReport::kStatsValueNamePacketsReceived, -1),
                GetValue<int>(&webrtc::StatsReport::Value::int_val, report,
                              webrtc::StatsReport::kStatsValueNamePacketsLost,
                              -1),
                GetValue<int>(&webrtc::StatsReport::Value::int_val, report,
                              webrtc::StatsReport::kStatsValueNameFirsSent, -1),
                GetValue<int>(&webrtc::StatsReport::Value::int_val, report,
                              webrtc::StatsReport::kStatsValueNamePlisSent, -1),
                GetValue<int>(&webrtc::StatsReport::Value::int_val, report,
                              webrtc::StatsReport::kStatsValueNameNacksSent,
                              -1),
                GetValue<int>(
                    &webrtc::StatsReport::Value::int_val, report,
                    webrtc::StatsReport::kStatsValueNameFrameHeightReceived,
                    -1),
                GetValue<int>(
                    &webrtc::StatsReport::Value::int_val, report,
                    webrtc::StatsReport::kStatsValueNameFrameWidthReceived, -1),
                GetValue<int>(
                    &webrtc::StatsReport::Value::int_val, report,
                    webrtc::StatsReport::kStatsValueNameFrameRateReceived, -1),
                GetValue<int>(
                    &webrtc::StatsReport::Value::int_val, report,
                    webrtc::StatsReport::kStatsValueNameFrameRateOutput, -1),
                GetValue<int>(
                    &webrtc::StatsReport::Value::int_val, report,
                    webrtc::StatsReport::kStatsValueNameCurrentDelayMs, -1),
                report->FindValue(webrtc::StatsReport::kStatsValueNameCodecName)
                    ->string_val(),
                GetValue<int>(
                    &webrtc::StatsReport::Value::int_val, report,
                    webrtc::StatsReport::kStatsValueNameJitterBufferMs, -1)));
        connection_stats->video_receiver_reports.push_back(
            std::move(video_recv_report_ptr));
        break;
      }
      case REPORT_VIDEO_SENDER:
      {
        adapt_reason =
            static_cast<int32_t>(VideoSenderReport::AdaptReason::kUnknown);
        if (report
                ->FindValue(
                    webrtc::StatsReport::kStatsValueNameCpuLimitedResolution)
                ->bool_val())
          adapt_reason = static_cast<int32_t>(
              VideoSenderReport::AdaptReason::kCpuLimitation);
        else if (report
                     ->FindValue(webrtc::StatsReport::
                                     kStatsValueNameBandwidthLimitedResolution)
                     ->bool_val())
          adapt_reason = static_cast<int32_t>(
              VideoSenderReport::AdaptReason::kBandwidthLimitation);
        else if (report
                     ->FindValue(webrtc::StatsReport::
                                     kStatsValueNameViewLimitedResolution)
                     ->bool_val())
          adapt_reason = static_cast<int32_t>(
              VideoSenderReport::AdaptReason::kViewLimitation);
        std::unique_ptr<VideoSenderReport> video_send_report_ptr(
            new VideoSenderReport(
                GetValue<int64_t>(
                    &webrtc::StatsReport::Value::int64_val, report,
                    webrtc::StatsReport::kStatsValueNameBytesSent, -1),
                GetValue<int>(&webrtc::StatsReport::Value::int_val, report,
                              webrtc::StatsReport::kStatsValueNamePacketsSent,
                              -1),
                GetValue<int>(&webrtc::StatsReport::Value::int_val, report,
                              webrtc::StatsReport::kStatsValueNamePacketsLost,
                              -1),
                GetValue<int>(&webrtc::StatsReport::Value::int_val, report,
                              webrtc::StatsReport::kStatsValueNameFirsReceived,
                              -1),
                GetValue<int>(&webrtc::StatsReport::Value::int_val, report,
                              webrtc::StatsReport::kStatsValueNamePlisReceived,
                              -1),
                GetValue<int>(&webrtc::StatsReport::Value::int_val, report,
                              webrtc::StatsReport::kStatsValueNameNacksReceived,
                              -1),
                GetValue<int>(
                    &webrtc::StatsReport::Value::int_val, report,
                    webrtc::StatsReport::kStatsValueNameFrameHeightSent, -1),
                GetValue<int>(
                    &webrtc::StatsReport::Value::int_val, report,
                    webrtc::StatsReport::kStatsValueNameFrameWidthSent, -1),
                GetValue<int>(&webrtc::StatsReport::Value::int_val, report,
                              webrtc::StatsReport::kStatsValueNameFrameRateSent,
                              -1),
                adapt_reason,
                GetValue<int>(
                    &webrtc::StatsReport::Value::int_val, report,
                    webrtc::StatsReport::kStatsValueNameAdaptationChanges, -1),
                GetValue<int64_t>(&webrtc::StatsReport::Value::int64_val,
                                  report,
                                  webrtc::StatsReport::kStatsValueNameRtt, -1),
                report->FindValue(webrtc::StatsReport::kStatsValueNameCodecName)
                    ->string_val()));
        connection_stats->video_sender_reports.push_back(
            std::move(video_send_report_ptr));
        break;
      }
      case REPORT_VIDEO_BWE:
      {
        connection_stats->video_bandwidth_stats.available_send_bandwidth =
            GetValue<int>(
                &webrtc::StatsReport::Value::int_val, report,
                webrtc::StatsReport::kStatsValueNameAvailableSendBandwidth, -1);
        connection_stats->video_bandwidth_stats.available_receive_bandwidth =
            GetValue<int>(
                &webrtc::StatsReport::Value::int_val, report,
                webrtc::StatsReport::kStatsValueNameAvailableReceiveBandwidth,
                -1);
        connection_stats->video_bandwidth_stats.transmit_bitrate =
            GetValue<int>(&webrtc::StatsReport::Value::int_val, report,
                          webrtc::StatsReport::kStatsValueNameTransmitBitrate,
                          -1);
        connection_stats->video_bandwidth_stats.retransmit_bitrate =
            GetValue<int>(&webrtc::StatsReport::Value::int_val, report,
                          webrtc::StatsReport::kStatsValueNameRetransmitBitrate,
                          -1);
        break;
      }
      default:
        break;
      }
    }

    on_complete_(connection_stats);
  }
}

IceCandidateType FunctionalStatsObserver::GetCandidateType(const std::string& type) {
  // |type| may be values defines in statscollector.cc, STATSREPORT_*_TYPE
  if (type == "host") {
    return IceCandidateType::kHost;
  }
  if (type == "serverreflexive") {
    return IceCandidateType::kSrflx;
  }
  if (type == "peerreflexive") {
    return IceCandidateType::kPrflx;
  }
  if (type == "relayed") {
    return IceCandidateType::kRelay;
  }
  RTC_DCHECK(false);
  return IceCandidateType::kUnknown;
}

TransportProtocolType FunctionalStatsObserver::GetTransportProtocolType(const std::string& protocol) {
  if (protocol == cricket::UDP_PROTOCOL_NAME) {
    return TransportProtocolType::kUdp;
  }
  if (protocol == cricket::TCP_PROTOCOL_NAME) {
    return TransportProtocolType::kTcp;
  }
  RTC_DCHECK(false);
  return TransportProtocolType::kUnknown;
}

std::shared_ptr<IceCandidateReport> FunctionalStatsObserver::GetIceCandidateReport(const webrtc::StatsReport& report){
   std::shared_ptr<IceCandidateReport> candidate_report_ptr(new IceCandidateReport(
      report.id()->ToString(),
      report.FindValue(webrtc::StatsReport::kStatsValueNameCandidateIPAddress)->string_val(),
      static_cast<uint16_t>(std::stoi(report.FindValue(webrtc::StatsReport::kStatsValueNameCandidatePortNumber)->string_val())),
      GetTransportProtocolType(report.FindValue(webrtc::StatsReport::kStatsValueNameCandidateTransportType)->string_val()),
      GetCandidateType(std::string(report.FindValue(webrtc::StatsReport::kStatsValueNameCandidateType)->static_string_val())),
      report.FindValue(webrtc::StatsReport::kStatsValueNameCandidatePriority)->int_val()));
   return candidate_report_ptr;
}

FunctionalStatsObserver::ReportType FunctionalStatsObserver::GetReportType(
    const webrtc::StatsReport* report) {
  // Check if it's ssrc report.
  bool isSending = 0;
  bool isVideo = 0;
  bool isSSRC = false;
  bool isBWE = false;
  if (report->type() == webrtc::StatsReport::kStatsReportTypeSsrc) {
    isSSRC = true;
    if (report->FindValue(
            webrtc::StatsReport::kStatsValueNameBytesSent))  // this is sending
      isSending = true;

    if (report->FindValue(webrtc::StatsReport::kStatsValueNameFrameWidthSent) ||
        report->FindValue(
            webrtc::StatsReport::kStatsValueNameFrameWidthReceived))
      isVideo = true;

  } else if (report->type() == webrtc::StatsReport::kStatsReportTypeBwe) {
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

}
}
