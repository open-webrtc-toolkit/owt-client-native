/*
 * Copyright © 2016 Intel Corporation. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WOOGEEN_BASE_CONNECTIONSTATS_H_
#define WOOGEEN_BASE_CONNECTIONSTATS_H_

#include <chrono>
#include <map>

namespace woogeen {
namespace base {

/// Define adapt reason
enum AdaptReason {
  /// Adapt for CPU limitation
  ADAPT_CPU_LIMITATION = 1,
  /// Adapt for bandwidth limitation
  ADAPT_BANDWIDTH_LIMITATION,
  /// Adapt for view limitation
  ADAPT_VIEW_LIMITATION,
  /// Unknown reason
  ADAPT_UNKNOWN = 99,
};

/// Define audio sender report
struct AudioSenderReport {
  AudioSenderReport(int64_t bytes_sent, int32_t packets_sent,
                    int32_t packets_lost, int64_t rtt_ms, std::string codec_name)
      : bytes_sent(bytes_sent), packets_sent(packets_sent), packets_lost(packets_lost)
      , rtt_ms(rtt_ms), codec_name(codec_name) {}
  /// Audio bytes sent
  int64_t bytes_sent;
  /// Audio packets sent
  int32_t packets_sent;
  /// Audio packets lost during sending
  int32_t packets_lost;
  /// RTT for audio sending with unit of millisecond
  int64_t rtt_ms;
  /// Audio codec name for sending
  std::string codec_name;
};

/// Define audio receiver report
struct AudioReceiverReport {
  AudioReceiverReport(int64_t bytes_rcvd, int32_t packets_rcvd,
                      int32_t packets_lost, int32_t delay_estimated_ms, std::string codec_name)
      : bytes_rcvd(bytes_rcvd), packets_rcvd(packets_rcvd), packets_lost(packets_lost)
      , delay_estimated_ms(delay_estimated_ms), codec_name(codec_name) {}
  /// Audio bytes received
  int64_t bytes_rcvd;
  /// Audio packets received
  int32_t packets_rcvd;
  /// Audio packets lost during receiving
  int32_t packets_lost;
  /// Audio delay estimated with unit of millisecond
  int32_t delay_estimated_ms;
  /// Audio codec name for receiving
  std::string codec_name;
};

/// Define video sender report
struct VideoSenderReport {
  VideoSenderReport(int64_t bytes_sent, int32_t packets_sent, int32_t packets_lost,
                    int32_t firs_rcvd, int32_t plis_rcvd, int32_t nacks_rcvd, int32_t sent_frame_height,
                    int32_t sent_frame_width, int32_t framerate_sent, AdaptReason last_adapt_reason,
                    int32_t adapt_changes, int64_t rtt_ms, std::string codec_name)
      : bytes_sent(bytes_sent), packets_sent(packets_sent), packets_lost(packets_lost)
      , firs_rcvd(firs_rcvd), plis_rcvd(plis_rcvd), nacks_rcvd(nacks_rcvd), sent_frame_height(sent_frame_height)
      , sent_frame_width(sent_frame_width), framerate_sent(framerate_sent), last_adapt_reason(last_adapt_reason)
      , adapt_changes(adapt_changes), rtt_ms(rtt_ms), codec_name(codec_name) {}
  /// Video bytes sent
  int64_t bytes_sent;
  /// Video packets sent
  int32_t packets_sent;
  /// Video packets lost during sending
  int32_t packets_lost;
  /// Number of FIR received
  int32_t firs_rcvd;
  /// Number of PLI received
  int32_t plis_rcvd;
  /// Number of NACK received
  int32_t nacks_rcvd;
  /// Video frame height sent
  int32_t sent_frame_height;
  /// Video frame width sent
  int32_t sent_frame_width;
  /// Video framerate sent
  int32_t framerate_sent;
  /// Video adapt reason
  AdaptReason last_adapt_reason;
  /// Video adapt changes
  int32_t adapt_changes;
  /// RTT for video sending with unit of millisecond
  int64_t rtt_ms;
  /// Video codec name for sending
  std::string codec_name;
};

/// Define video receiver report
struct VideoReceiverReport {
  VideoReceiverReport(int64_t bytes_rcvd, int32_t packets_rcvd, int32_t packets_lost,
                      int32_t firs_sent, int32_t plis_sent, int32_t nacks_sent, int32_t rcvd_frame_height,
                      int32_t rcvd_frame_width, int32_t framerate_rcvd, int32_t framerate_output,
                      int32_t current_delay_ms, std::string codec_name)
      : bytes_rcvd(bytes_rcvd), packets_rcvd(packets_rcvd), packets_lost(packets_lost)
      , firs_sent(firs_sent), plis_sent(plis_sent), nacks_sent(nacks_sent), rcvd_frame_width(rcvd_frame_width)
      , rcvd_frame_height(rcvd_frame_height), framerate_rcvd(framerate_rcvd), framerate_output(framerate_output)
      , current_delay_ms(current_delay_ms), codec_name(codec_name) {}
  /// Video bytes received
  int64_t bytes_rcvd;
  /// Video packets received
  int32_t packets_rcvd;
  /// Video packets lost during receiving
  int32_t packets_lost;
  /// Number of FIR sent
  int32_t firs_sent;
  /// Number of PLI sent
  int32_t plis_sent;
  /// Number of PLI sent
  int32_t nacks_sent;
  /// Video frame width received
  int32_t rcvd_frame_width;
  /// Video frame height received
  int32_t rcvd_frame_height;
  /// Video framerate received
  int32_t framerate_rcvd;
  /// Video framerate output
  int32_t framerate_output;
  /// Current video delay with unit of millisecond
  int32_t current_delay_ms;
  /// Video codec name for receiving
  std::string codec_name;
};

/// Define video bandwidth statistics
struct VideoBandwidthStats {
  VideoBandwidthStats() : available_send_bandwidth(0), available_receive_bandwidth(0)
                        , transmit_bitrate(0), retransmit_bitrate(0) {}
  /// Available video bandwidth for sending
  int32_t available_send_bandwidth;
  /// Available video bandwidth for receiving
  int32_t available_receive_bandwidth;
  /// Video bitrate of transmit
  int32_t transmit_bitrate;
  /// Video bitrate of retransmit
  int32_t retransmit_bitrate;
};

typedef std::unique_ptr<AudioSenderReport> AudioSenderReportPtr;
typedef std::map<std::string, AudioSenderReportPtr> AudioSenderReports;
typedef std::unique_ptr<AudioReceiverReport> AudioReceiverReportPtr;
typedef std::map<std::string, AudioReceiverReportPtr> AudioReceiverReports;
typedef std::unique_ptr<VideoSenderReport> VideoSenderReportPtr;
typedef std::map<std::string, VideoSenderReportPtr> VideoSenderReports;
typedef std::unique_ptr<VideoReceiverReport> VideoReceiverReportPtr;
typedef std::map<std::string, VideoReceiverReportPtr> VideoReceiverReports;

/// Connection statistics
struct ConnectionStats {
  ConnectionStats() {}

  /// Time stamp of connection statistics generation
  std::chrono::system_clock::time_point time_stamp = std::chrono::system_clock::now();
  /// Video bandwidth statistics
  VideoBandwidthStats video_bandwidth_stats;
  /// Audio sender reports
  AudioSenderReports audio_sender_reports;
  /// Audio receiver reports
  AudioReceiverReports audio_receiver_reports;
  /// Video sender reports
  VideoSenderReports video_sender_reports;
  /// Video receiver reports
  VideoReceiverReports video_receiver_reports;
};
};
};

#endif  // WOOGEEN_BASE_CONNECTIONSTATS_H_
