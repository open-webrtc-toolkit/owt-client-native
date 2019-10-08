// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_CONNECTIONSTATS_H_
#define OWT_BASE_CONNECTIONSTATS_H_
#include <chrono>
#include <memory>
#include <string>
#include <vector>
#include "owt/base/commontypes.h"
#include "owt/base/network.h"
namespace owt {
namespace base {
/// Define audio sender report
struct AudioSenderReport {
  AudioSenderReport(int64_t bytes_sent, int32_t packets_sent,
                    int32_t packets_lost, int64_t round_trip_time, std::string codec_name)
      : bytes_sent(bytes_sent), packets_sent(packets_sent), packets_lost(packets_lost)
      , round_trip_time(round_trip_time), codec_name(codec_name) {}
  /// Audio bytes sent
  int64_t bytes_sent;
  /// Audio packets sent
  int32_t packets_sent;
  /// Audio packets lost during sending
  int32_t packets_lost;
  /// RTT for audio sending with unit of millisecond
  int64_t round_trip_time;
  /// Audio codec name for sending
  std::string codec_name;
};
/// Define audio receiver report
struct AudioReceiverReport {
  AudioReceiverReport(int64_t bytes_rcvd, int32_t packets_rcvd,
                      int32_t packets_lost, int32_t estimated_delay, std::string codec_name)
      : bytes_rcvd(bytes_rcvd), packets_rcvd(packets_rcvd), packets_lost(packets_lost)
      , estimated_delay(estimated_delay), codec_name(codec_name) {}
  /// Audio bytes received
  int64_t bytes_rcvd;
  /// Audio packets received
  int32_t packets_rcvd;
  /// Audio packets lost during receiving
  int32_t packets_lost;
  /// Audio delay estimated with unit of millisecond
  int32_t estimated_delay;
  /// Audio codec name for receiving
  std::string codec_name;
};
/// Define video sender report
struct VideoSenderReport {
  VideoSenderReport(int64_t bytes_sent, int32_t packets_sent, int32_t packets_lost,
                    int32_t fir_count, int32_t pli_count, int32_t nack_count, int32_t sent_frame_height,
                    int32_t sent_frame_width, int32_t framerate_sent, int32_t last_adapt_reason,
                    int32_t adapt_changes, int64_t round_trip_time, std::string codec_name)
      : bytes_sent(bytes_sent), packets_sent(packets_sent), packets_lost(packets_lost)
      , fir_count(fir_count), pli_count(pli_count), nack_count(nack_count), frame_resolution_sent(Resolution(sent_frame_width, sent_frame_height))
      , framerate_sent(framerate_sent), last_adapt_reason(last_adapt_reason)
      , adapt_changes(adapt_changes), round_trip_time(round_trip_time), codec_name(codec_name) {}
  /// Define adapt reason
  enum class AdaptReason : int32_t {
    kUnknown = 0,
    /// Adapt for CPU limitation
    kCpuLimitation = 1,
    /// Adapt for bandwidth limitation
    kBandwidthLimitation = 2,
    /// Adapt for view limitation
    kViewLimitation = 4,
  };
  /// Video bytes sent
  int64_t bytes_sent;
  /// Video packets sent
  int32_t packets_sent;
  /// Video packets lost during sending
  int32_t packets_lost;
  /// Number of FIR received
  int32_t fir_count;
  /// Number of PLI received
  int32_t pli_count;
  /// Number of NACK received
  int32_t nack_count;
  /// Video frame resolution sent
  Resolution frame_resolution_sent;
  /// Video framerate sent
  int32_t framerate_sent;
  /// Video adapt reason
  int32_t last_adapt_reason;
  /// Video adapt changes
  int32_t adapt_changes;
  /// RTT for video sending with unit of millisecond
  int64_t round_trip_time;
  /// Video codec name for sending
  std::string codec_name;
};
/// Define video receiver report
struct VideoReceiverReport {
  VideoReceiverReport(int64_t bytes_rcvd, int32_t packets_rcvd, int32_t packets_lost,
                      int32_t fir_count, int32_t pli_count, int32_t nack_count, int32_t rcvd_frame_height,
                      int32_t rcvd_frame_width, int32_t framerate_rcvd, int32_t framerate_output,
                      int32_t delay, std::string codec_name, int32_t jitter)
      : bytes_rcvd(bytes_rcvd), packets_rcvd(packets_rcvd), packets_lost(packets_lost)
      , fir_count(fir_count), pli_count(pli_count), nack_count(nack_count)
      , frame_resolution_rcvd(Resolution(rcvd_frame_width, rcvd_frame_height)), framerate_output(framerate_output)
      , delay(delay), codec_name(codec_name), jitter(jitter) {}
  /// Video bytes received
  int64_t bytes_rcvd;
  /// Video packets received
  int32_t packets_rcvd;
  /// Video packets lost during receiving
  int32_t packets_lost;
  /// Number of FIR sent
  int32_t fir_count;
  /// Number of PLI sent
  int32_t pli_count;
  /// Number of PLI sent
  int32_t nack_count;
  /// Video frame resolution received
  Resolution frame_resolution_rcvd;
  /// Video framerate received
  int32_t framerate_rcvd;
  /// Video framerate output
  int32_t framerate_output;
  /// Current video delay with unit of millisecond
  int32_t delay;
  /// Video codec name for receiving
  std::string codec_name;
  /// Packet Jitter measured in milliseconds
  int32_t jitter;
};
/// Define video bandwidth statistoms
struct VideoBandwidthStats {
  VideoBandwidthStats() : available_send_bandwidth(0), available_receive_bandwidth(0)
                        , transmit_bitrate(0), retransmit_bitrate(0)
                        , target_encoding_bitrate(0), actual_encoding_bitrate(0) {}
  /// Available video bandwidth for sending, unit: bps
  int32_t available_send_bandwidth;
  /// Available video bandwidth for receiving, unit: bps
  int32_t available_receive_bandwidth;
  /// Video bitrate of transmit, unit: bps
  int32_t transmit_bitrate;
  /// Video bitrate of retransmit, unit: bps
  int32_t retransmit_bitrate;
  /// Target encoding bitrate, unit: bps
  int32_t target_encoding_bitrate;
  /// Actual encoding bitrate, unit: bps
  int32_t actual_encoding_bitrate;
};
/// Define ICE candidate report
struct IceCandidateReport {
  IceCandidateReport(const std::string& id,
                     const std::string& ip,
                     const uint16_t port,
                     TransportProtocolType protocol,
                     IceCandidateType candidate_type,
                     int32_t priority)
      : id(id),
        ip(ip),
        port(port),
        protocol(protocol),
        candidate_type(candidate_type),
        priority(priority) {}
  virtual ~IceCandidateReport() {};
  /// The ID of this report
  std::string id;
  /// The IP address of the candidate
  std::string ip;
  /// The port number of the candidate
  uint16_t port;
  /// Transport protocol.
  TransportProtocolType protocol;
  /// Candidate type
  IceCandidateType candidate_type;
  /// Calculated as defined in RFC5245
  int32_t priority;
};
/// Define ICE candidate pair report.
struct IceCandidatePairReport {
  IceCandidatePairReport(
      const std::string& id,
      const bool is_active,
      std::shared_ptr<IceCandidateReport> local_ice_candidate,
      std::shared_ptr<IceCandidateReport> remote_ice_candidate)
      : id(id),
        is_active(is_active),
        local_ice_candidate(local_ice_candidate),
        remote_ice_candidate(remote_ice_candidate) {}
  /// The ID of this report.
  std::string id;
  /// Indicate whether transport is active.
  bool is_active;
  /// Local candidate of this pair.
  std::shared_ptr<IceCandidateReport> local_ice_candidate;
  /// Remote candidate of this pair.
  std::shared_ptr<IceCandidateReport> remote_ice_candidate;
};
typedef std::unique_ptr<AudioSenderReport> AudioSenderReportPtr;
typedef std::vector<AudioSenderReportPtr> AudioSenderReports;
typedef std::unique_ptr<AudioReceiverReport> AudioReceiverReportPtr;
typedef std::vector<AudioReceiverReportPtr> AudioReceiverReports;
typedef std::unique_ptr<VideoSenderReport> VideoSenderReportPtr;
typedef std::vector<VideoSenderReportPtr> VideoSenderReports;
typedef std::unique_ptr<VideoReceiverReport> VideoReceiverReportPtr;
typedef std::vector<VideoReceiverReportPtr> VideoReceiverReports;
typedef std::shared_ptr<IceCandidateReport> IceCandidateReportPtr;
typedef std::vector<IceCandidateReportPtr> IceCandidateReports;
typedef std::shared_ptr<IceCandidatePairReport> IceCandidatePairPtr;
typedef std::vector<IceCandidatePairPtr> IceCandidatePairReports;
/// Connection statistoms
struct ConnectionStats {
  ConnectionStats() {}
  /// Time stamp of connection statistowt generation
  std::chrono::system_clock::time_point time_stamp = std::chrono::system_clock::now();
  /// Video bandwidth statistoms
  VideoBandwidthStats video_bandwidth_stats;
  /// Audio sender reports
  AudioSenderReports audio_sender_reports;
  /// Audio receiver reports
  AudioReceiverReports audio_receiver_reports;
  /// Video sender reports
  VideoSenderReports video_sender_reports;
  /// Video receiver reports
  VideoReceiverReports video_receiver_reports;
  /// Local ICE candidate reports
  IceCandidateReports local_ice_candidate_reports;
  /// Remote ICE candidate reports
  IceCandidateReports remote_ice_candidate_reports;
  /// ICE candidate pair reports
  IceCandidatePairReports ice_candidate_pair_reports;
};
} // namespace base
} // namespace owt
#endif  // OWT_BASE_CONNECTIONSTATS_H_
