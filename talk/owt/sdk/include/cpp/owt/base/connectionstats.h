// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_CONNECTIONSTATS_H_
#define OWT_BASE_CONNECTIONSTATS_H_

#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "owt/base/commontypes.h"
#include "owt/base/export.h"
#include "owt/base/network.h"

namespace owt {
namespace base {
// RTC-spec compliant statistics interfaces.
// https://w3c.github.io/webrtc-pc/#idl-def-rtcdatachannelstate
struct OWT_EXPORT RTCDataChannelState {
  static const char* const kConnecting;
  static const char* const kOpen;
  static const char* const kClosing;
  static const char* const kClosed;
};

// https://w3c.github.io/webrtc-stats/#dom-rtcstatsicecandidatepairstate
struct OWT_EXPORT RTCStatsIceCandidatePairState {
  static const char* const kFrozen;
  static const char* const kWaiting;
  static const char* const kInProgress;
  static const char* const kFailed;
  static const char* const kSucceeded;
};

// https://w3c.github.io/webrtc-pc/#rtcicecandidatetype-enum
struct OWT_EXPORT RTCIceCandidateType {
  static const char* const kHost;
  static const char* const kSrflx;
  static const char* const kPrflx;
  static const char* const kRelay;
};

// https://w3c.github.io/webrtc-pc/#idl-def-rtcdtlstransportstate
struct OWT_EXPORT RTCDtlsTransportState {
  static const char* const kNew;
  static const char* const kConnecting;
  static const char* const kConnected;
  static const char* const kClosed;
  static const char* const kFailed;
};

// |RTCMediaStreamTrackStats::kind| is not an enum in the spec but the only
// valid values are "audio" and "video".
// https://w3c.github.io/webrtc-stats/#dom-rtcmediastreamtrackstats-kind
struct OWT_EXPORT RTCMediaStreamTrackKind {
  static const char* const kAudio;
  static const char* const kVideo;
};

// https://w3c.github.io/webrtc-stats/#dom-rtcnetworktype
struct OWT_EXPORT RTCNetworkType {
  static const char* const kBluetooth;
  static const char* const kCellular;
  static const char* const kEthernet;
  static const char* const kWifi;
  static const char* const kWimax;
  static const char* const kVpn;
  static const char* const kUnknown;
};

// https://w3c.github.io/webrtc-stats/#dom-rtcqualitylimitationreason
struct OWT_EXPORT RTCQualityLimitationReason {
  static const char* const kNone;
  static const char* const kCpu;
  static const char* const kBandwidth;
  static const char* const kOther;
};

// https://webrtc.org/experiments/rtp-hdrext/video-content-type/
struct OWT_EXPORT RTCContentType {
  static const char* const kUnspecified;
  static const char* const kScreenshare;
};

/*!
 * \sa https://w3c.github.io/webrtc-stats/#rtcstatstype-str*
 */
struct OWT_EXPORT RTCStatsType {
  static const char* const kCodec;
  static const char* const kInboundRTP;
  static const char* const kOutboundRTP;
  static const char* const kRemoteInboundRTP;
  static const char* const kRemoteOutboundRTP;
  static const char* const kMediaSource;
  static const char* const kCsrc;
  static const char* const kPeerConnection;
  static const char* const kDataChannel;
  static const char* const kStream;
  static const char* const kTrack;
  static const char* const kTranseiver;
  static const char* const kSender;
  static const char* const kReceiver;
  static const char* const kTransport;
  static const char* const kSctpTransport;
  static const char* const kCandidatePair;
  static const char* const kLocalCandidate;
  static const char* const kRemoteCandidate;
  static const char* const kCertificate;
  static const char* const kIceServer;
};

/*!
 * \sa https://w3c.github.io/webrtc-stats/#idl-def-rtcstats*
 */
class OWT_EXPORT RTCStats {
 public:
  RTCStats(const std::string& type, const std::string& id, int64_t timestamp_us)
      : type(type), id(id), timestamp_us(timestamp_us) {}
  RTCStats(const RTCStats& other) {
    type = other.type;
    id = other.id;
    timestamp_us = other.timestamp_us;
  }
  virtual ~RTCStats() = default;

  // Downcasts the stats object to an |RTCStats| subclass |T|. TODO: DCHECKs
  // that the object is of type |T|.
  template <typename T>
  const T& cast_to() const {
    return static_cast<const T&>(*this);
  }

  std::string type;
  std::string id;
  int64_t timestamp_us;
};

class OWT_EXPORT RTCStatsReport {
 public:
  RTCStatsReport() {}
  virtual ~RTCStatsReport() {}
  typedef std::map<std::string, std::unique_ptr<const owt::base::RTCStats>>
      StatsMap;

  class OWT_EXPORT ConstIterator {
   public:
    ConstIterator(ConstIterator&& other);
    ~ConstIterator();

    ConstIterator& operator++();
    ConstIterator& operator++(int);
    const RTCStats& operator*() const;
    const RTCStats* operator->() const;
    bool operator==(const ConstIterator& other) const;
    bool operator!=(const ConstIterator& other) const;

   private:
    friend class owt::base::RTCStatsReport;
    ConstIterator(owt::base::RTCStatsReport* report,
                  StatsMap::const_iterator it);

    StatsMap::const_iterator it_;
  };

  void AddStats(std::unique_ptr<const owt::base::RTCStats> stats);
  const owt::base::RTCStats* Get(const std::string& id) const;
  size_t size() const { return stats_map.size(); }

  // Gets the stat object of type |T| by ID, where |T| is any class descending
  // from |RTCStats|.
  // Returns null if there is no stats object for the given ID or it is the
  // wrong type.
  template <typename T>
  const T* GetAs(const std::string& id) const {
    const RTCStats* stats = Get(id);
    if (!stats || stats->type != T::kType) {
      return nullptr;
    }
    return &stats->cast_to<const T>();
  }

  // Removes the stats object from the report, returning ownership of it or null
  // if there is no object with |id|.
  std::unique_ptr<const owt::base::RTCStats> Take(const std::string& id);
  // Takes ownership of all the stats in |victim|, leaving it empty.
  void TakeMembersFrom(std::shared_ptr<owt::base::RTCStatsReport> victim);

  // Stats iterators. Stats are ordered lexicographically on |RTCStats::id|.
  ConstIterator begin() const;
  ConstIterator end() const;

  // Gets the subset of stats that are of type |T|, where |T| is any class
  // descending from |RTCStats|.
  template <typename T>
  std::vector<const T*> GetStatsOfType() const {
    std::vector<const T*> stats_of_type;
    for (const RTCStats& stats : *this) {
      if (stats.type == T::kType)
        stats_of_type.push_back(&stats.cast_to<const T>());
    }
    return stats_of_type;
  }

 private:
  StatsMap stats_map;
};

/*!
 * \sa https://w3c.github.io/webrtc-stats/#certificatestats-dict*
 */
class OWT_EXPORT RTCCertificateStats : public RTCStats {
 public:
  RTCCertificateStats(const std::string& id,
                      int64_t timestamp,
                      const std::string& fingerprint,
                      const std::string& fingerprint_algorithm,
                      const std::string& base64_certificate,
                      const std::string& issuer_certificate_id);
  RTCCertificateStats(const RTCCertificateStats& other);
  ~RTCCertificateStats();

  std::string fingerprint;
  // The hash funciton used to calculate the certificate fingerprint.
  // For insance, "sha-256".
  std::string fingerprint_algorithm;
  // The DER-encoded based-64 rep of the certifcate.
  std::string base64_certificate;
  // The id of the stats object that contains the next certificate
  // in the certificate chain.
  std::string issuer_certificate_id;
};

/*!
 * \sa https://w3c.github.io/webrtc-stats/#codec-dict*
 */
class OWT_EXPORT RTCCodecStats final : public RTCStats {
 public:
  RTCCodecStats(const std::string& id,
                int64_t timestamp,
                uint32_t payload_type,
                const std::string& mime_type,
                uint32_t clock_rate,
                uint32_t channels,
                const std::string& sdp_fmtp_line);
  RTCCodecStats(const RTCCodecStats& other);
  ~RTCCodecStats() override;

  uint32_t payload_type;
  // Non-populated member: codec_type. Commonly it's both encode & decode
  // so by spec not included.
  // Non-populated member: transportId. By spec it referes to the id of
  // RTCTransportStats object which uses current codec.
  std::string mime_type;
  uint32_t clock_rate;
  uint32_t channels;
  std::string sdp_fmtp_line;
};

/*!
 * \sa https://w3c.github.io/webrtc-stats/#dcstats-dict*
 */
class OWT_EXPORT RTCDataChannelStats final : public RTCStats {
 public:
  RTCDataChannelStats(const std::string& id,
                      int64_t timestamp,
                      const std::string& label,
                      const std::string& protocol,
                      int32_t datachannelid,
                      const std::string& state,
                      uint32_t messages_sent,
                      uint64_t bytes_sent,
                      uint32_t messages_received,
                      uint64_t bytes_received);
  RTCDataChannelStats(const RTCDataChannelStats& other);
  ~RTCDataChannelStats() override;

  // Label of the datachannel.
  std::string label;
  std::string protocol;
  // The "id" attribute ofthe datachannel
  int32_t data_channel_identifier;
  // TODO: Support enum types? "RTCStatsMember<RTCDataChannelState>"?
  // The "readyState" of the DataChannel object
  std::string state;
  // The total number of API level message events sent.
  uint32_t messages_sent;
  // Total number of payload bytes sent, not including headers and paddings.
  uint64_t bytes_sent;
  // The total number of API level message events received.
  uint32_t messages_received;
  // The total number of payload bytes received, not including headers and
  // paddings
  uint64_t bytes_received;
};

/*!
 * \sa https://w3c.github.io/webrtc-stats/#candidatepair-dict*
 */
class OWT_EXPORT RTCIceCandidatePairStats final : public RTCStats {
 public:
  RTCIceCandidatePairStats(const std::string& id,
                           int64_t timestamp,
                           const std::string& transport_id,
                           const std::string& local_candidate_id,
                           const std::string& remote_candidate_id,
                           const std::string& state,
                           uint64_t priority,
                           bool nominated,
                           bool writable,
                           uint64_t bytes_sent,
                           uint64_t bytes_received,
                           double total_round_trip_time,
                           double current_round_trip_time,
                           double available_outgoing_bitrate,
                           double avialable_incoming_bitrate,
                           uint64_t requests_received,
                           uint64_t requests_sent,
                           uint64_t responses_received,
                           uint64_t responses_sent,
                           uint64_t consent_requests_sent);
  RTCIceCandidatePairStats(const RTCIceCandidatePairStats& other);
  ~RTCIceCandidatePairStats() override;

  // The uinque transportId of the RTCTransportStats associated with this
  // candidate pair.
  std::string transport_id;
  std::string local_candidate_id;
  std::string remote_candidate_id;
  std::string state;
  // obsoleted in latest spec.
  uint64_t priority;
  bool nominated;

  // Below two are not included in spec. You can ignore them.
  bool writable;

  // TODO: by spec below we should populate thse two stats:
  // uint64_t packets_sent, uint64_t packets_received.

  // Total bytes sent on this candidate pair not including headers/paddings/ice
  // checks.
  uint64_t bytes_sent;
  // Total bytes received on this candidate pair not including
  // headers/paddings/ice checks.
  uint64_t bytes_received;

  // TODO: not populated: lastPacketSentTimestamp/lastPacketReceivedTimestamp
  // TODO: not populated: lastRequestTimestamp/lastResponseTimestamp

  // Sum of all roundtrip time measurements in seconds since beginning of the
  // session. Refer to spec on how to use this value to calculate the average
  // round trip time.
  double total_round_trip_time;
  // Latest round trip time measured in seconds, computed from STUN connectivity
  // checks.
  double current_round_trip_time;
  // The bandwidth estimation result for all the outgoing RTP streams using this
  // candidate pair. Implementation that does not support sender side BWE must
  // leave this value as undefined.
  double available_outgoing_bitrate;
  // Refer to spec for more details.
  double available_incoming_bitrate;
  // total number of connectivity check requests received.
  uint64_t requests_received;
  // total number of connectivity check requests sent.
  uint64_t requests_sent;
  // total number of connectivity check response received.
  uint64_t responses_received;
  // total number of connectivity check response sent.
  uint64_t responses_sent;
  uint64_t consent_requests_sent;
};

/*!
 * \sa https://w3c.github.io/webrtc-stats/#icecandidate-dict*
 */
// TODO: |RTCStatsCollector| only collects candidates that are part of
// ice candidate pairs, but there could be candidates not paired with anything.
// TODO: Add the stats of STUN binding requests (keepalives) and collect
// them in the new PeerConnection::GetStats.
class OWT_EXPORT RTCIceCandidateStats : public RTCStats {
 public:
  RTCIceCandidateStats(const std::string& type,
                       const std::string& id,
                       int64_t timestamp,
                       const std::string& transport_id,
                       bool is_remote,
                       const std::string& network_type,
                       const std::string& ip,
                       int32_t port,
                       const std::string& protocol,
                       const std::string& relay_protocol,
                       const std::string& candidate_type,
                       int32_t priority,
                       const std::string& url,
                       bool deleted);
  RTCIceCandidateStats(const RTCIceCandidateStats& other);
  ~RTCIceCandidateStats() override;

  // Transport id of the RTCTransportStats associated with this
  // candidate.
  std::string transport_id;
  bool is_remote;
  std::string network_type;
  std::string ip;
  int32_t port;
  std::string protocol;
  std::string relay_protocol;
  // TODO: Support enum types? "RTCStatsMember<RTCIceCandidateType>"?
  std::string candidate_type;
  int32_t priority;
  // TODO: Not collected by |RTCStatsCollector|.
  std::string url;
  // obsoleted in latest spec.
  bool deleted;  // = false
};

/*!
 * \sa https://w3c.github.io/webrtc-stats/#rtcstatstype-str*
 */
class OWT_EXPORT RTCLocalIceCandidateStats final : RTCIceCandidateStats {
 public:
  RTCLocalIceCandidateStats(const std::string& id,
                            int64_t timestamp,
                            const std::string& transport_id,
                            bool is_remote,
                            const std::string& network_type,
                            const std::string& ip,
                            int32_t port,
                            const std::string& protocol,
                            const std::string& relay_protocol,
                            const std::string& candidate_type,
                            int32_t priority,
                            const std::string& url,
                            bool deleted);
  RTCLocalIceCandidateStats(const RTCLocalIceCandidateStats& other);
  ~RTCLocalIceCandidateStats() override;
};

/*!
 * \sa https://w3c.github.io/webrtc-stats/#rtcstatstype-str*
 */
class OWT_EXPORT RTCRemoteIceCandidateStats final : public RTCIceCandidateStats {
 public:
  RTCRemoteIceCandidateStats(const std::string& id,
                             int64_t timestamp,
                             const std::string& transport_id,
                             bool is_remote,
                             const std::string& network_type,
                             const std::string& ip,
                             int32_t port,
                             const std::string& protocol,
                             const std::string& relay_protocol,
                             const std::string& candidate_type,
                             int32_t priority,
                             const std::string& url,
                             bool deleted);
  RTCRemoteIceCandidateStats(const RTCRemoteIceCandidateStats& other);
  ~RTCRemoteIceCandidateStats() override;
};

/*!
 * \sa https://w3c.github.io/webrtc-stats/#msstats-dict*
 */
// Obsoleted due to sender/receiver/transceiver stats being better
// fits to describe the modern RTCPeerConnection model (unified plan).
class OWT_EXPORT RTCMediaStreamStats final : public RTCStats {
 public:
  RTCMediaStreamStats(const std::string& id,
                      int64_t timestamp,
                      const std::string& stream_identifier,
                      const std::vector<std::string>& track_ids);
  RTCMediaStreamStats(const RTCMediaStreamStats& other);
  ~RTCMediaStreamStats() override;

  std::string stream_identifier;
  std::vector<std::string> track_ids;
};


/*!
 * \sa https://w3c.github.io/webrtc-stats/#msstats-dict*
 */
// This is a combined implementation of RTCInboundRtpStreamStats
// and RTCOutboundRtpStreamStats.
class OWT_EXPORT RTCMediaStreamTrackStats final : public RTCStats {
 public:
  RTCMediaStreamTrackStats(const std::string& id,
                           int64_t timestamp,
                           std::string track_identifier,
                           std::string media_source_id,
                           bool remote_source,
                           bool ended,
                           bool detached,
                           std::string kind,
                           double jitter_buffer_delay,
                           uint64_t jitter_buffer_emitted_count,
                           uint32_t frame_width,
                           uint32_t frame_height,
                           uint32_t frames_sent,
                           uint32_t huge_frames_sent,
                           uint32_t frames_received,
                           uint32_t frames_decoded,
                           uint32_t frames_dropped,
                           double audio_level,
                           double total_audio_energy,
                           double echo_return_loss,
                           double echo_return_loss_enhancement,
                           uint64_t total_samples_received,
                           double total_samples_duration,
                           uint64_t concealed_samples,
                           uint64_t silent_concealed_samples,
                           uint64_t concealment_events,
                           uint64_t inserted_samples_for_deceleration,
                           uint64_t removed_samples_for_acceleration,
                           uint64_t jitter_buffer_flushes,
                           uint64_t delayed_packet_outage_samples,
                           double relative_packet_arrival_delay,
                           uint32_t interruption_count,
                           double total_interruption_duration,
                           uint32_t freeze_count,
                           uint32_t pause_count,
                           double total_freezes_duration,
                           double total_pauses_duration,
                           double total_frames_duration,
                           double sum_squared_frame_durations);
  RTCMediaStreamTrackStats(const RTCMediaStreamTrackStats& other);
  ~RTCMediaStreamTrackStats() override;

  std::string track_identifier;
  std::string media_source_id;
  bool remote_source;
  bool ended;
  // TODO: |RTCStatsCollector| does not return stats for detached tracks.
  bool detached;
  // See |RTCMediaStreamTrackKind| for valid values.
  std::string kind;
  double jitter_buffer_delay;
  uint64_t jitter_buffer_emitted_count;
  // Video-only members
  uint32_t frame_width;
  uint32_t frame_height;
  // TODO: Not collected by |RTCStatsCollector|.
  double frames_per_second;
  uint32_t frames_sent;
  uint32_t huge_frames_sent;
  uint32_t frames_received;
  uint32_t frames_decoded;
  uint32_t frames_dropped;
  // Audio-only members
  double audio_level;         // Receive-only
  double total_audio_energy;  // Receive-only
  double echo_return_loss;
  double echo_return_loss_enhancement;
  uint64_t total_samples_received;
  double total_samples_duration;  // Receive-only
  uint64_t concealed_samples;
  uint64_t silent_concealed_samples;
  uint64_t concealment_events;
  uint64_t inserted_samples_for_deceleration;
  uint64_t removed_samples_for_acceleration;
  // Non-standard audio-only member
  // TODO: Add description to standard.
  uint64_t jitter_buffer_flushes;
  uint64_t delayed_packet_outage_samples;
  double relative_packet_arrival_delay;
  // Non-standard metric showing target delay of jitter buffer.
  // This value is increased by the target jitter buffer delay every time a
  // sample is emitted by the jitter buffer. The added target is the target
  // delay, in seconds, at the time that the sample was emitted from the jitter
  // buffer.
  // Currently it is implemented only for audio.
  // TODO: implement for video streams when will be requested.
  double jitter_buffer_target_delay;
  uint32_t interruption_count;
  double total_interruption_duration;
  // Non-standard video-only members.
  // https://henbos.github.io/webrtc-provisional-stats/#RTCVideoReceiverStats-dict*
  uint32_t freeze_count;
  uint32_t pause_count;
  double total_freezes_duration;
  double total_pauses_duration;
  double total_frames_duration;
  double sum_squared_frame_durations;
};

/*!
 * \sa https://w3c.github.io/webrtc-stats/#pcstats-dict
 */
class OWT_EXPORT RTCPeerConnectionStats final : public RTCStats {
 public:
  RTCPeerConnectionStats(const std::string& id,
                         int64_t timestamp,
                         uint32_t data_channels_opened,
                         uint32_t data_channels_closed);
  RTCPeerConnectionStats(const RTCPeerConnectionStats& other);
  ~RTCPeerConnectionStats() override;

  uint32_t data_channels_opened;
  uint32_t data_channels_closed;
  // data_channels_requested & data_channels_accepted not populated.
};

/*!
 * \sa https://w3c.github.io/webrtc-stats/#streamstats-dict*
 */
class OWT_EXPORT RTCRTPStreamStats : public RTCStats {
 public:
  RTCRTPStreamStats(const std::string& type,
                    const std::string& id,
                    int64_t timestamp,
                    uint32_t ssrc,
                    const std::string& media_type,
                    const std::string& kind,
                    const std::string& track_id,
                    const std::string& transport_id,
                    const std::string& codec_id,
                    uint32_t fir_count,
                    uint32_t pli_count,
                    uint32_t nack_count,
                    uint64_t qp_sum);
  RTCRTPStreamStats(const RTCRTPStreamStats& other);
  ~RTCRTPStreamStats() override;

  uint32_t ssrc;
  // TODO: Remote case not supported by |RTCStatsCollector|.
  bool is_remote;          // = false
  std::string media_type;  // renamed to kind.
  std::string kind;
  std::string track_id;
  std::string transport_id;
  std::string codec_id;
  // FIR and PLI counts are only defined for |media_type == "video"|.
  uint32_t fir_count;
  uint32_t pli_count;
  // TODO: NACK count should be collected by |RTCStatsCollector| for both
  // audio and video but is only defined in the "video" case.
  uint32_t nack_count;
  // SLI count is only defined for |media_type == "video"|.
  uint32_t sli_count;
  uint64_t qp_sum;
};

/*!
 * \sa https://w3c.github.io/webrtc-stats/#inboundrtpstats-dict*
 */
// TODO: Support the remote case |is_remote = true|.
class OWT_EXPORT RTCInboundRTPStreamStats final : public RTCRTPStreamStats {
 public:
  RTCInboundRTPStreamStats(const std::string& id,
                           int64_t timestamp,
                           uint32_t ssrc,
                           const std::string& media_type,
                           const std::string& kind,
                           const std::string& track_id,
                           const std::string& transport_id,
                           const std::string& codec_id,
                           uint32_t fir_count,
                           uint32_t pli_count,
                           uint32_t nack_count,
                           uint64_t qp_sum,
                           uint32_t packets_received,
                           uint64_t fec_packets_received,
                           uint64_t fec_packets_discarded,
                           uint64_t bytes_received,
                           uint64_t header_bytes_received,
                           int32_t packets_lost,
                           double last_packet_received_timestamp,
                           double jitter,
                           uint32_t packets_discarded,
                           uint32_t frames_decoded,
                           uint32_t key_frames_decoded,
                           double total_decode_time,
                           double total_inter_frame_delay,
                           double total_squared_inter_frame_delay,
                           const std::string& content_type,
                           double estimated_playout_timestamp,
                           const std::string& decoder_implementation);
  RTCInboundRTPStreamStats(const RTCInboundRTPStreamStats& other);
  ~RTCInboundRTPStreamStats() override;

  uint32_t packets_received;
  uint64_t fec_packets_received;
  uint64_t fec_packets_discarded;
  uint64_t bytes_received;
  uint64_t header_bytes_received;
  int32_t packets_lost;  // Signed per RFC 3550
  double last_packet_received_timestamp;
  // TODO: Collect and populate this value for both "audio" and "video",
  // currently not collected for "video".
  double jitter;
  // TODO: Collect and populate this value.
  double round_trip_time;
  // TODO: Collect and populate this value.
  uint32_t packets_discarded;
  // TODO: Collect and populate this value.
  uint32_t packets_repaired;
  // TODO: Collect and populate this value.
  uint32_t burst_packets_lost;
  // TODO: Collect and populate this value.
  uint32_t burst_packets_discarded;
  // TODO: Collect and populate this value.
  uint32_t burst_loss_count;
  // TODO: Collect and populate this value.
  uint32_t burst_discard_count;
  // TODO: Collect and populate this value.
  double burst_loss_rate;
  // TODO: Collect and populate this value.
  double burst_discard_rate;
  // TODO: Collect and populate this value.
  double gap_loss_rate;
  // TODO: Collect and populate this value.
  double gap_discard_rate;
  uint32_t frames_decoded;
  uint32_t frames_rendered;
  uint32_t key_frames_decoded;
  double total_decode_time;
  double total_inter_frame_delay;
  double total_squared_inter_frame_delay;
  // https://henbos.github.io/webrtc-provisional-stats/#dom-rtcinboundrtpstreamstats-contenttype
  std::string content_type;
  // TODO: Currently only populated if audio/video sync is enabled.
  double estimated_playout_timestamp;
  // TODO: This is only implemented for video; implement it for audio as
  // well.
  std::string decoder_implementation;
};

/*!
 * \sa https://w3c.github.io/webrtc-stats/#outboundrtpstats-dict*
 */
// TODO: Support the remote case |is_remote = true|.
class OWT_EXPORT RTCOutboundRTPStreamStats final : public RTCRTPStreamStats {
 public:
  RTCOutboundRTPStreamStats(const std::string& id,
                            int64_t timestamp,
                            uint32_t ssrc,
                            const std::string& media_type,
                            const std::string& kind,
                            const std::string& track_id,
                            const std::string& transport_id,
                            const std::string& codec_id,
                            uint32_t fir_count,
                            uint32_t pli_count,
                            uint32_t nack_count,
                            uint64_t qp_sum,
                            const std::string& media_source_id,
                            const std::string& remote_id,
                            uint32_t packets_sent,
                            uint64_t retransmitted_packets_sent,
                            uint64_t bytes_sent,
                            uint64_t header_bytes_sent,
                            uint64_t retransmitted_bytes_sent,
                            double target_bitrate,
                            uint32_t frames_encoded,
                            uint32_t key_frames_encoded,
                            double total_encode_time,
                            uint64_t total_encoded_bytes_target,
                            double total_packet_send_delay,
                            const std::string& quality_limitation_reason,
                            uint32_t quality_limitation_resolution_changes,
                            const std::string& content_type,
                            const std::string& encoder_implementation);
  RTCOutboundRTPStreamStats(const RTCOutboundRTPStreamStats& other);
  ~RTCOutboundRTPStreamStats() override;

  std::string media_source_id;
  std::string remote_id;
  uint32_t packets_sent;
  uint64_t retransmitted_packets_sent;
  uint64_t bytes_sent;
  uint64_t header_bytes_sent;
  uint64_t retransmitted_bytes_sent;
  // TODO: Collect and populate this value.
  double target_bitrate;
  uint32_t frames_encoded;
  uint32_t key_frames_encoded;
  double total_encode_time;
  uint64_t total_encoded_bytes_target;
  // TODO: This is only implemented for video;
  // implement it for audio as well.
  double total_packet_send_delay;
  // Enum type RTCQualityLimitationReason
  // TODO: Also expose
  // qualityLimitationDurations. Requires RTCStatsMember support for
  // "record<DOMString, double>"
  std::string quality_limitation_reason;
  // https://w3c.github.io/webrtc-stats/#dom-rtcoutboundrtpstreamstats-qualitylimitationresolutionchanges
  uint32_t quality_limitation_resolution_changes;
  // https://henbos.github.io/webrtc-provisional-stats/#dom-rtcoutboundrtpstreamstats-contenttype
  std::string content_type;
  // TODO: This is only implemented for video; implement it for audio as
  // well.
  std::string encoder_implementation;
};

/*!
 * \sa https://w3c.github.io/webrtc-stats/#remoteinboundrtpstats-dict*
 */
// TODO: Refactor the stats dictionaries to have
// the same hierarchy as in the spec; implement RTCReceivedRtpStreamStats.
// Several metrics are shared between "outbound-rtp", "remote-inbound-rtp",
// "inbound-rtp" and "remote-outbound-rtp". In the spec there is a hierarchy of
// dictionaries that minimizes defining the same metrics in multiple places.
// From JavaScript this hierarchy is not observable and the spec's hierarchy is
// purely editorial. In C++ non-final classes in the hierarchy could be used to
// refer to different stats objects within the hierarchy.
// https://w3c.github.io/webrtc-stats/#remoteinboundrtpstats-dict*
// RTCRemoteInboundRtpStreamsStats represents the measurements by the remote
// outgoing rtp stream at the sending endpoint. (By spec it should inherit
// from RTCReceivedRtpStreamStats and include extra properties including
// following: localId, roundTripTime, totalRoundTripTime, fractionLost,
// reportsReceived & roundTripTimeMeasurements.)
class OWT_EXPORT RTCRemoteInboundRtpStreamStats final : public RTCStats {
 public:
  RTCRemoteInboundRtpStreamStats(const std::string& id,
                                 int64_t timestamp,
                                 uint32_t ssrc,
                                 const std::string& kind,
                                 const std::string& transport_id,
                                 const std::string& codec_id,
                                 int32_t packets_lost,
                                 double jitter,
                                 const std::string& local_id,
                                 double round_trip_time);
  RTCRemoteInboundRtpStreamStats(const RTCRemoteInboundRtpStreamStats& other);
  ~RTCRemoteInboundRtpStreamStats() override;

  // In the spec RTCRemoteInboundRtpStreamStats inherits from RTCRtpStreamStats
  // and RTCReceivedRtpStreamStats. The members here are listed based on where
  // they are defined in the spec.
  // RTCRtpStreamStats
  uint32_t ssrc;
  std::string kind;
  std::string transport_id;
  std::string codec_id;
  // RTCReceivedRtpStreamStats
  int32_t packets_lost;
  double jitter;
  // TODO: The following RTCReceivedRtpStreamStats metrics should also be
  // implemented: packetsReceived, packetsDiscarded, packetsRepaired,
  // burstPacketsLost, burstPacketsDiscarded, burstLossCount, burstDiscardCount,
  // burstLossRate, burstDiscardRate, gapLossRate and gapDiscardRate.
  // RTCRemoteInboundRtpStreamStats
  std::string local_id;
  double round_trip_time;
  // TODO: The following RTCRemoteInboundRtpStreamStats metric should also
  // be implemented: fractionLost.
};

/*!
 * \sa https://w3c.github.io/webrtc-stats/#dom-rtcmediasourcestats
 */
class OWT_EXPORT RTCMediaSourceStats : public RTCStats {
 public:
  RTCMediaSourceStats(const std::string& type,
                      const std::string& id,
                      int64_t timestamp,
                      const std::string& track_identifier,
                      const std::string& kind);
  RTCMediaSourceStats(const RTCMediaSourceStats& other);
  ~RTCMediaSourceStats() override;

  std::string track_identifier;
  std::string kind;
};

/*!
 * \sa  https://w3c.github.io/webrtc-stats/#dom-rtcaudiosourcestats
 */
class OWT_EXPORT RTCAudioSourceStats final : public RTCMediaSourceStats {
 public:
  RTCAudioSourceStats(const std::string& id,
                      int64_t timestamp,
                      const std::string& track_identifier,
                      const std::string& kind,
                      double audio_level,
                      double total_audio_engergy,
                      double total_samples_duration);
  RTCAudioSourceStats(const RTCAudioSourceStats& other);
  ~RTCAudioSourceStats() override;

  double audio_level;
  double total_audio_energy;
  double total_samples_duration;
};

/*!
 * \sa  https://w3c.github.io/webrtc-stats/#dom-rtcvideosourcestats
 */
class OWT_EXPORT RTCVideoSourceStats final : public RTCMediaSourceStats {
 public:
  RTCVideoSourceStats(const std::string& id,
                      int64_t timestamp,
                      const std::string& track_identifier,
                      const std::string& kind,
                      uint32_t width,
                      uint32_t height,
                      uint32_t frames,
                      uint32_t frames_per_second);
  RTCVideoSourceStats(const RTCVideoSourceStats& other);
  ~RTCVideoSourceStats() override;

  uint32_t width;
  uint32_t height;
  // TODO: Implement this metric.
  uint32_t frames;
  uint32_t frames_per_second;
};

/*!
 * \sa  https://w3c.github.io/webrtc-stats/#transportstats-dict*
 */
class OWT_EXPORT RTCTransportStats final : public RTCStats {
 public:
  RTCTransportStats(const std::string& id,
                    int64_t timestamp,
                    uint64_t bytes_sent,
                    uint64_t bytes_received,
                    const std::string& rtcp_transport_stats_id,
                    const std::string& dtls_state,
                    const std::string& selected_candidate_pair_id,
                    const std::string& local_certificate_id,
                    const std::string& remote_certificate_id,
                    const std::string& tls_version,
                    const std::string& dtls_cipher,
                    const std::string& srtp_cipher,
                    uint32_t selected_candidate_pair_changes);
  RTCTransportStats(const RTCTransportStats& other);
  ~RTCTransportStats() override;

  uint64_t bytes_sent;
  uint64_t bytes_received;
  std::string rtcp_transport_stats_id;
  // TODO: Support enum types? "RTCStatsMember<RTCDtlsTransportState>"?
  std::string dtls_state;
  std::string selected_candidate_pair_id;
  std::string local_certificate_id;
  std::string remote_certificate_id;
  std::string tls_version;
  std::string dtls_cipher;
  std::string srtp_cipher;
  uint32_t selected_candidate_pair_changes;
};

/// Define audio sender report
struct OWT_EXPORT AudioSenderReport {
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
struct OWT_EXPORT AudioReceiverReport {
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
struct OWT_EXPORT VideoSenderReport {
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
struct OWT_EXPORT VideoReceiverReport {
  VideoReceiverReport(int64_t bytes_rcvd, int32_t packets_rcvd, int32_t packets_lost,
                      int32_t fir_count, int32_t pli_count, int32_t nack_count, int32_t rcvd_frame_height,
                      int32_t rcvd_frame_width, int32_t framerate_rcvd, int32_t framerate_output,
                      int32_t delay, std::string codec_name, int32_t jitter)
      : bytes_rcvd(bytes_rcvd), packets_rcvd(packets_rcvd), packets_lost(packets_lost)
      , fir_count(fir_count), pli_count(pli_count), nack_count(nack_count)
      , frame_resolution_rcvd(Resolution(rcvd_frame_width, rcvd_frame_height))
      , framerate_rcvd(framerate_rcvd), framerate_output(framerate_output)
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
struct OWT_EXPORT VideoBandwidthStats {
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
struct OWT_EXPORT IceCandidateReport {
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
  virtual ~IceCandidateReport() {}
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
struct OWT_EXPORT IceCandidatePairReport {
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
struct OWT_EXPORT ConnectionStats {
  ConnectionStats() {}
  /// Time stamp of connection statistics generation
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
