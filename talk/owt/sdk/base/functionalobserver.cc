// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "talk/owt/sdk/base/functionalobserver.h"
#include <memory>
#include "api/stats/rtc_stats.h"
#include "api/stats/rtcstats_objects.h"

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
std::shared_ptr<IceCandidateReport> FunctionalStatsObserver::GetIceCandidateReport(
    const webrtc::StatsReport& report){
  if (report.FindValue(
          webrtc::StatsReport::kStatsValueNameCandidateIPAddress) == nullptr ||
      report.FindValue(
          webrtc::StatsReport::kStatsValueNameCandidatePortNumber) == nullptr ||
      report.FindValue(
          webrtc::StatsReport::kStatsValueNameCandidateTransportType) ==
          nullptr ||
      report.FindValue(webrtc::StatsReport::kStatsValueNameCandidateType) ==
          nullptr ||
      report.FindValue(webrtc::StatsReport::kStatsValueNameCandidatePriority) ==
          nullptr)
    return nullptr;

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

rtc::scoped_refptr<FunctionalStandardRTCStatsCollectorCallback>
FunctionalStandardRTCStatsCollectorCallback::Create(
    std::function<void(std::shared_ptr<owt::base::RTCStatsReport>)>
        on_stats_delivered) {
  return new rtc::RefCountedObject<FunctionalStandardRTCStatsCollectorCallback>(
      std::move(on_stats_delivered));
}

void FunctionalStandardRTCStatsCollectorCallback::OnStatsDelivered(
    const rtc::scoped_refptr<const webrtc::RTCStatsReport>& report) {
  if (!on_stats_delivered_)
    return;

  std::shared_ptr<owt::base::RTCStatsReport> reports =
      std::make_shared<owt::base::RTCStatsReport>();

  // Create the owt version of RTCStatsReport and send to the
  // owt_stats_delivered_ callback.
  for (const auto& stats : *report) {
    if (strcmp(stats.type(), RTCStatsType::kDataChannel) == 0) {
      auto& webrtc_stats = stats.cast_to<webrtc::RTCDataChannelStats>();
      std::unique_ptr<owt::base::RTCDataChannelStats> stat =
          std::make_unique<owt::base::RTCDataChannelStats>(
              stats.id(), webrtc_stats.timestamp_us(),
              webrtc_stats.label.is_defined()
                  ? webrtc_stats.label.ValueToString()
                  : "",
              webrtc_stats.protocol.is_defined()
                  ? webrtc_stats.protocol.ValueToString()
                  : "",
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, datachannelid, int32_t,
                                         (-1)),
              webrtc_stats.state.is_defined()
                  ? webrtc_stats.state.ValueToString()
                  : "",
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, messages_sent, uint32_t,
                                         0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, bytes_sent, uint64_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, messages_received,
                                         uint32_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, bytes_received, uint64_t,
                                         0));
      reports->AddStats(std::move(stat));
    } else if (strcmp(stats.type(), RTCStatsType::kCandidatePair) == 0) {
      auto& webrtc_stats = stats.cast_to<webrtc::RTCIceCandidatePairStats>();
      std::unique_ptr<owt::base::RTCIceCandidatePairStats> stat =
          std::make_unique<owt::base::RTCIceCandidatePairStats>(
              webrtc_stats.id(), webrtc_stats.timestamp_us(),
              webrtc_stats.transport_id.is_defined()
                  ? webrtc_stats.transport_id.ValueToString()
                  : "",
              webrtc_stats.local_candidate_id.is_defined()
                  ? webrtc_stats.local_candidate_id.ValueToString()
                  : "",
              webrtc_stats.remote_candidate_id.is_defined()
                  ? webrtc_stats.remote_candidate_id.ValueToString()
                  : "",
              webrtc_stats.state.is_defined()
                  ? webrtc_stats.state.ValueToString()
                  : "",
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, priority, uint64_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, nominated, bool, false),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, writable, bool, false),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, readable, bool, false),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, bytes_sent, uint64_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, bytes_received, uint64_t,
                                         0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, total_round_trip_time,
                                         double, (-1)),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, current_round_trip_time,
                                         double, (-1)),
              OWT_STATS_VALUE_OR_DEFAULT(
                  webrtc_stats, available_outgoing_bitrate, double, (-1)),
              OWT_STATS_VALUE_OR_DEFAULT(
                  webrtc_stats, available_incoming_bitrate, double, (-1)),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, requests_received,
                                         uint64_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, requests_sent, uint64_t,
                                         0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, responses_received,
                                         uint64_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, responses_sent, uint64_t,
                                         0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, retransmissions_received,
                                         uint64_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, retransmissions_sent,
                                         uint64_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(
                  webrtc_stats, consent_requests_received, uint64_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, consent_requests_sent,
                                         uint64_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(
                  webrtc_stats, consent_responses_received, uint64_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, consent_responses_sent,
                                         uint64_t, 0));
      reports->AddStats(std::move(stat));
    } else if (strcmp(stats.type(), RTCStatsType::kTrack) == 0) {
      auto& webrtc_stats = stats.cast_to<webrtc::RTCMediaStreamTrackStats>();
      std::unique_ptr<owt::base::RTCMediaStreamTrackStats> stat =
          std::make_unique<owt::base::RTCMediaStreamTrackStats>(
              webrtc_stats.id(), webrtc_stats.timestamp_us(),
              webrtc_stats.track_identifier.is_defined()
                  ? webrtc_stats.track_identifier.ValueToString()
                  : "",
              webrtc_stats.media_source_id.is_defined()
                  ? webrtc_stats.media_source_id.ValueToString()
                  : "",
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, remote_source, bool,
                                         false),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, ended, bool, false),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, detached, bool, false),
              webrtc_stats.kind.is_defined() ? webrtc_stats.kind.ValueToString()
                                             : "",
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, jitter_buffer_delay,
                                         double, -1),
              OWT_STATS_VALUE_OR_DEFAULT(
                  webrtc_stats, jitter_buffer_emitted_count, uint64_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, frame_width, uint32_t,
                                         0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, frame_height, uint32_t,
                                         0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, frames_per_second,
                                         double, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, frames_sent, uint32_t,
                                         0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, huge_frames_sent,
                                         uint32_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, frames_received,
                                         uint32_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, frames_decoded, uint32_t,
                                         0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, frames_dropped, uint32_t,
                                         0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, frames_corrupted,
                                         uint32_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, partial_frames_lost,
                                         uint32_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, full_frames_lost,
                                         uint32_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, audio_level, double, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, total_audio_energy,
                                         double, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, echo_return_loss, double,
                                         0),
              OWT_STATS_VALUE_OR_DEFAULT(
                  webrtc_stats, echo_return_loss_enhancement, double, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, total_samples_received,
                                         uint64_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, total_samples_duration,
                                         double, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, concealed_samples,
                                         uint64_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, silent_concealed_samples,
                                         uint64_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, concealment_events,
                                         uint64_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(
                  webrtc_stats, inserted_samples_for_deceleration, uint64_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(
                  webrtc_stats, removed_samples_for_acceleration, uint64_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, jitter_buffer_flushes,
                                         uint64_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(
                  webrtc_stats, delayed_packet_outage_samples, uint64_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(
                  webrtc_stats, relative_packet_arrival_delay, double, -1),
              OWT_STATS_VALUE_OR_DEFAULT(
                  webrtc_stats, jitter_buffer_target_delay, double, -1),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, interruption_count,
                                         uint32_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(
                  webrtc_stats, total_interruption_duration, double, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, freeze_count, uint32_t,
                                         0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, pause_count, uint32_t,
                                         0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, total_freezes_duration,
                                         double, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, total_pauses_duration,
                                         double, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, total_frames_duration,
                                         double, 0),
              OWT_STATS_VALUE_OR_DEFAULT(
                  webrtc_stats, sum_squared_frame_durations, double, 0));
      reports->AddStats(std::move(stat));
    } else if (strcmp(stats.type(), RTCStatsType::kInboundRTP) == 0) {
      auto& webrtc_stats = stats.cast_to<webrtc::RTCInboundRTPStreamStats>();
      std::unique_ptr<owt::base::RTCInboundRTPStreamStats> stat =
          std::make_unique<owt::base::RTCInboundRTPStreamStats>(
              webrtc_stats.id(), webrtc_stats.timestamp_us(),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, ssrc, uint32_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, is_remote, bool, false),
              webrtc_stats.media_type.is_defined()
                  ? webrtc_stats.media_type.ValueToString()
                  : "",
              webrtc_stats.kind.is_defined() ? webrtc_stats.kind.ValueToString()
                                             : "",
              webrtc_stats.track_id.is_defined()
                  ? webrtc_stats.track_id.ValueToString()
                  : "",
              webrtc_stats.transport_id.is_defined()
                  ? webrtc_stats.transport_id.ValueToString()
                  : "",
              webrtc_stats.codec_id.is_defined()
                  ? webrtc_stats.codec_id.ValueToString()
                  : "",
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, fir_count, uint32_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, pli_count, uint32_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, nack_count, uint32_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, sli_count, uint32_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, qp_sum, uint64_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, packets_received,
                                         uint32_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, fec_packets_received,
                                         uint64_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, fec_packets_discarded,
                                         uint64_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, bytes_received, uint64_t,
                                         0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, header_bytes_received,
                                         uint64_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, packets_lost, int32_t,
                                         0),
              OWT_STATS_VALUE_OR_DEFAULT(
                  webrtc_stats, last_packet_received_timestamp, double, -1),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, jitter, double, -1),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, round_trip_time, double,
                                         -1),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, packets_discarded,
                                         uint32_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, packets_repaired,
                                         uint32_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, burst_packets_lost,
                                         uint32_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, burst_packets_discarded,
                                         uint32_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, burst_loss_count,
                                         uint32_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, burst_discard_count,
                                         uint32_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, burst_loss_rate, double,
                                         0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, burst_discard_rate,
                                         double, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, gap_loss_rate, double,
                                         0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, gap_discard_rate, double,
                                         0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, frames_decoded, uint32_t,
                                         0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, key_frames_decoded,
                                         uint32_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, total_decode_time,
                                         double, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, total_inter_frame_delay,
                                         double, 0),
              OWT_STATS_VALUE_OR_DEFAULT(
                  webrtc_stats, total_squared_inter_frame_delay, double, 0),
              webrtc_stats.content_type.is_defined()
                  ? webrtc_stats.content_type.ValueToString()
                  : "",
              OWT_STATS_VALUE_OR_DEFAULT(
                  webrtc_stats, estimated_playout_timestamp, double, 0),
              webrtc_stats.decoder_implementation.is_defined()
                  ? webrtc_stats.decoder_implementation.ValueToString()
                  : "");
      reports->AddStats(std::move(stat));
    } else if (strcmp(stats.type(), RTCStatsType::kOutboundRTP) == 0) {
      auto& webrtc_stats = stats.cast_to<webrtc::RTCOutboundRTPStreamStats>();
      std::unique_ptr<owt::base::RTCOutboundRTPStreamStats> stat =
          std::make_unique<owt::base::RTCOutboundRTPStreamStats>(
              webrtc_stats.id(), webrtc_stats.timestamp_us(),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, ssrc, uint32_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, is_remote, bool, false),
              webrtc_stats.media_type.is_defined()
                  ? webrtc_stats.media_type.ValueToString()
                  : "",
              webrtc_stats.kind.is_defined() ? webrtc_stats.kind.ValueToString()
                                             : "",
              webrtc_stats.track_id.is_defined()
                  ? webrtc_stats.track_id.ValueToString()
                  : "",
              webrtc_stats.transport_id.is_defined()
                  ? webrtc_stats.transport_id.ValueToString()
                  : "",
              webrtc_stats.codec_id.is_defined()
                  ? webrtc_stats.codec_id.ValueToString()
                  : "",
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, fir_count, uint32_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, pli_count, uint32_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, nack_count, uint32_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, sli_count, uint32_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, qp_sum, uint64_t, 0),
              webrtc_stats.media_source_id.is_defined()
                  ? webrtc_stats.media_source_id.ValueToString()
                  : "",
              webrtc_stats.remote_id.is_defined()
                  ? webrtc_stats.remote_id.ValueToString()
                  : "",
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, packets_sent, uint32_t,
                                         0),
              OWT_STATS_VALUE_OR_DEFAULT(
                  webrtc_stats, retransmitted_packets_sent, uint64_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, bytes_sent, uint64_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, header_bytes_sent,
                                         uint64_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, retransmitted_bytes_sent,
                                         uint64_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, target_bitrate, double,
                                         0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, frames_encoded, uint32_t,
                                         0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, key_frames_encoded,
                                         uint32_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, total_encode_time,
                                         double, 0),
              OWT_STATS_VALUE_OR_DEFAULT(
                  webrtc_stats, total_encoded_bytes_target, uint64_t, 0),
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats, total_packet_send_delay,
                                         double, 0),
              webrtc_stats.quality_limitation_reason.is_defined()
                  ? webrtc_stats.quality_limitation_reason.ValueToString()
                  : "",
              OWT_STATS_VALUE_OR_DEFAULT(webrtc_stats,
                                         quality_limitation_resolution_changes,
                                         uint32_t, 0),
              webrtc_stats.content_type.is_defined()
                  ? webrtc_stats.content_type.ValueToString()
                  : "",
              webrtc_stats.encoder_implementation.is_defined()
                  ? webrtc_stats.encoder_implementation.ValueToString()
                  : "");
      reports->AddStats(std::move(stat));
    }
  }
  if (reports->size() > 0) {
    on_stats_delivered_(reports);
  }
}

} // namespace base
} // namespace owt
