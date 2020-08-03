// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "talk/owt/sdk/base/functionalobserver.h"
namespace owt {
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
    webrtc::RTCError error) {
  if (on_failure_ != nullptr) {
    on_failure_(error.message());
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
    webrtc::RTCError error) {
  if (on_failure_ != nullptr) {
    on_failure_(error.message());
  }
}

FunctionalSetRemoteDescriptionObserver::FunctionalSetRemoteDescriptionObserver(
    std::function<void(webrtc::RTCError error)> on_complete)
    : on_complete_(on_complete) {}

rtc::scoped_refptr<FunctionalSetRemoteDescriptionObserver>
FunctionalSetRemoteDescriptionObserver::Create(
    std::function<void(webrtc::RTCError error)> on_complete) {
  return new rtc::RefCountedObject<FunctionalSetRemoteDescriptionObserver>(
      on_complete);
}

void FunctionalSetRemoteDescriptionObserver::OnSetRemoteDescriptionComplete(
    webrtc::RTCError error) {
  if (on_complete_ != nullptr) {
    on_complete_(std::move(error));
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
                    webrtc::StatsReport::kStatsValueNameBytesReceived, 0),
                GetValue<int>(
                    &webrtc::StatsReport::Value::int_val, report,
                    webrtc::StatsReport::kStatsValueNamePacketsReceived, 0),
                GetValue<int>(&webrtc::StatsReport::Value::int_val, report,
                              webrtc::StatsReport::kStatsValueNamePacketsLost,
                              0),
                GetValue<int>(
                    &webrtc::StatsReport::Value::int_val, report,
                    webrtc::StatsReport::kStatsValueNameCurrentDelayMs, 0),
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
                    webrtc::StatsReport::kStatsValueNameBytesSent, 0),
                GetValue<int>(&webrtc::StatsReport::Value::int_val, report,
                              webrtc::StatsReport::kStatsValueNamePacketsSent,
                              0),
                GetValue<int>(&webrtc::StatsReport::Value::int_val, report,
                              webrtc::StatsReport::kStatsValueNamePacketsLost,
                              0),
                GetValue<int64_t>(&webrtc::StatsReport::Value::int64_val,
                                  report,
                                  webrtc::StatsReport::kStatsValueNameRtt, 0),
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
                    webrtc::StatsReport::kStatsValueNameBytesReceived, 0),
                GetValue<int>(
                    &webrtc::StatsReport::Value::int_val, report,
                    webrtc::StatsReport::kStatsValueNamePacketsReceived, 0),
                GetValue<int>(&webrtc::StatsReport::Value::int_val, report,
                              webrtc::StatsReport::kStatsValueNamePacketsLost,
                              0),
                GetValue<int>(&webrtc::StatsReport::Value::int_val, report,
                              webrtc::StatsReport::kStatsValueNameFirsSent, 0),
                GetValue<int>(&webrtc::StatsReport::Value::int_val, report,
                              webrtc::StatsReport::kStatsValueNamePlisSent, 0),
                GetValue<int>(&webrtc::StatsReport::Value::int_val, report,
                              webrtc::StatsReport::kStatsValueNameNacksSent,
                              0),
                GetValue<int>(
                    &webrtc::StatsReport::Value::int_val, report,
                    webrtc::StatsReport::kStatsValueNameFrameHeightReceived,
                    0),
                GetValue<int>(
                    &webrtc::StatsReport::Value::int_val, report,
                    webrtc::StatsReport::kStatsValueNameFrameWidthReceived, 0),
                GetValue<int>(
                    &webrtc::StatsReport::Value::int_val, report,
                    webrtc::StatsReport::kStatsValueNameFrameRateReceived, 0),
                GetValue<int>(
                    &webrtc::StatsReport::Value::int_val, report,
                    webrtc::StatsReport::kStatsValueNameFrameRateOutput, 0),
                GetValue<int>(
                    &webrtc::StatsReport::Value::int_val, report,
                    webrtc::StatsReport::kStatsValueNameCurrentDelayMs, 0),
                report->FindValue(webrtc::StatsReport::kStatsValueNameCodecName)
                    ->string_val(),
                // TODO: Jitter should be double.
                GetValue<int>(
                    &webrtc::StatsReport::Value::int_val, report,
                    webrtc::StatsReport::kStatsValueNameJitterBufferMs, 0)));
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
        std::unique_ptr<VideoSenderReport> video_send_report_ptr(
            new VideoSenderReport(
                GetValue<int64_t>(
                    &webrtc::StatsReport::Value::int64_val, report,
                    webrtc::StatsReport::kStatsValueNameBytesSent, 0),
                GetValue<int>(&webrtc::StatsReport::Value::int_val, report,
                              webrtc::StatsReport::kStatsValueNamePacketsSent,
                              0),
                GetValue<int>(&webrtc::StatsReport::Value::int_val, report,
                              webrtc::StatsReport::kStatsValueNamePacketsLost,
                              0),
                GetValue<int>(&webrtc::StatsReport::Value::int_val, report,
                              webrtc::StatsReport::kStatsValueNameFirsReceived,
                              0),
                GetValue<int>(&webrtc::StatsReport::Value::int_val, report,
                              webrtc::StatsReport::kStatsValueNamePlisReceived,
                              0),
                GetValue<int>(&webrtc::StatsReport::Value::int_val, report,
                              webrtc::StatsReport::kStatsValueNameNacksReceived,
                              0),
                GetValue<int>(
                    &webrtc::StatsReport::Value::int_val, report,
                    webrtc::StatsReport::kStatsValueNameFrameHeightSent, 0),
                GetValue<int>(
                    &webrtc::StatsReport::Value::int_val, report,
                    webrtc::StatsReport::kStatsValueNameFrameWidthSent, 0),
                GetValue<int>(&webrtc::StatsReport::Value::int_val, report,
                              webrtc::StatsReport::kStatsValueNameFrameRateSent,
                              0),
                adapt_reason,
                GetValue<int>(
                    &webrtc::StatsReport::Value::int_val, report,
                    webrtc::StatsReport::kStatsValueNameAdaptationChanges, 0),
                GetValue<int64_t>(&webrtc::StatsReport::Value::int64_val,
                                  report,
                                  webrtc::StatsReport::kStatsValueNameRtt, 0),
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
                webrtc::StatsReport::kStatsValueNameAvailableSendBandwidth, 0);
        connection_stats->video_bandwidth_stats.available_receive_bandwidth =
            GetValue<int>(
                &webrtc::StatsReport::Value::int_val, report,
                webrtc::StatsReport::kStatsValueNameAvailableReceiveBandwidth,
                0);
        connection_stats->video_bandwidth_stats.transmit_bitrate =
            GetValue<int>(&webrtc::StatsReport::Value::int_val, report,
                          webrtc::StatsReport::kStatsValueNameTransmitBitrate,
                          0);
        connection_stats->video_bandwidth_stats.retransmit_bitrate =
            GetValue<int>(&webrtc::StatsReport::Value::int_val, report,
                          webrtc::StatsReport::kStatsValueNameRetransmitBitrate,
                          0);
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
rtc::scoped_refptr<FunctionalNativeStatsObserver>
FunctionalNativeStatsObserver::Create(
    std::function<void(const webrtc::StatsReports& reports)> on_complete) {
  return new rtc::RefCountedObject<FunctionalNativeStatsObserver>(on_complete);
}
void FunctionalNativeStatsObserver::OnComplete(
    const webrtc::StatsReports& reports) {
  if (!on_complete_) {
    return;
  }
  on_complete_(reports);
}
}
}
