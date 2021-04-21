// Copyright (C) <2020> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include <map>
#include <type_traits>
#include <utility>
#include "rtc_base/checks.h"
#include "rtc_base/strings/string_builder.h"
#include "talk/owt/sdk/include/cpp/owt/base/connectionstats.h"

namespace owt {
namespace base {
const char* const RTCDataChannelState::kConnecting = "connecting";
const char* const RTCDataChannelState::kOpen = "open";
const char* const RTCDataChannelState::kClosing = "closing";
const char* const RTCDataChannelState::kClosed = "closed";

const char* const RTCStatsIceCandidatePairState::kFrozen = "frozen";
const char* const RTCStatsIceCandidatePairState::kWaiting = "waiting";
const char* const RTCStatsIceCandidatePairState::kInProgress = "in-progress";
const char* const RTCStatsIceCandidatePairState::kFailed = "failed";
const char* const RTCStatsIceCandidatePairState::kSucceeded = "succeeded";

// Strings defined in https://tools.ietf.org/html/rfc5245.
const char* const RTCIceCandidateType::kHost = "host";
const char* const RTCIceCandidateType::kSrflx = "srflx";
const char* const RTCIceCandidateType::kPrflx = "prflx";
const char* const RTCIceCandidateType::kRelay = "relay";

const char* const RTCDtlsTransportState::kNew = "new";
const char* const RTCDtlsTransportState::kConnecting = "connecting";
const char* const RTCDtlsTransportState::kConnected = "connected";
const char* const RTCDtlsTransportState::kClosed = "closed";
const char* const RTCDtlsTransportState::kFailed = "failed";

const char* const RTCMediaStreamTrackKind::kAudio = "audio";
const char* const RTCMediaStreamTrackKind::kVideo = "video";

// https://w3c.github.io/webrtc-stats/#dom-rtcnetworktype
const char* const RTCNetworkType::kBluetooth = "bluetooth";
const char* const RTCNetworkType::kCellular = "cellular";
const char* const RTCNetworkType::kEthernet = "ethernet";
const char* const RTCNetworkType::kWifi = "wifi";
const char* const RTCNetworkType::kWimax = "wimax";
const char* const RTCNetworkType::kVpn = "vpn";
const char* const RTCNetworkType::kUnknown = "unknown";

// https://w3c.github.io/webrtc-stats/#dom-rtcqualitylimitationreason
const char* const RTCQualityLimitationReason::kNone = "none";
const char* const RTCQualityLimitationReason::kCpu = "cpu";
const char* const RTCQualityLimitationReason::kBandwidth = "bandwidth";
const char* const RTCQualityLimitationReason::kOther = "other";

// https://webrtc.org/experiments/rtp-hdrext/video-content-type/
const char* const RTCContentType::kUnspecified = "unspecified";
const char* const RTCContentType::kScreenshare = "screenshare";

const char* const RTCStatsType::kCodec = "codec";
const char* const RTCStatsType::kInboundRTP = "inbound-rtp";
const char* const RTCStatsType::kOutboundRTP = "outbound-rtp";
const char* const RTCStatsType::kRemoteInboundRTP = "remote-inbound-rtp";
const char* const RTCStatsType::kRemoteOutboundRTP = "remote-outbound-rtp";
const char* const RTCStatsType::kMediaSource = "media-source";
const char* const RTCStatsType::kCsrc = "csrc";
const char* const RTCStatsType::kPeerConnection = "peer-connection";
const char* const RTCStatsType::kDataChannel = "data-channel";
const char* const RTCStatsType::kStream = "stream";
const char* const RTCStatsType::kTrack = "track";
const char* const RTCStatsType::kTranseiver = "transceiver";
const char* const RTCStatsType::kSender = "sender";
const char* const RTCStatsType::kReceiver = "receiver";
const char* const RTCStatsType::kTransport = "transport";
const char* const RTCStatsType::kSctpTransport = "sctp-transport";
const char* const RTCStatsType::kCandidatePair = "candidate-pair";
const char* const RTCStatsType::kLocalCandidate = "local-candidate";
const char* const RTCStatsType::kRemoteCandidate = "remote-candidate";
const char* const RTCStatsType::kCertificate = "certificate";
const char* const RTCStatsType::kIceServer = "ice-server";

RTCStatsReport::ConstIterator::ConstIterator(
    owt::base::RTCStatsReport* report,
    StatsMap::const_iterator it)
    : it_(it) {}

RTCStatsReport::ConstIterator::ConstIterator(ConstIterator&& other) = default;

RTCStatsReport::ConstIterator::~ConstIterator() {}

RTCStatsReport::ConstIterator& RTCStatsReport::ConstIterator::operator++() {
  ++it_;
  return *this;
}

RTCStatsReport::ConstIterator& RTCStatsReport::ConstIterator::operator++(int) {
  return ++(*this);
}

const RTCStats& RTCStatsReport::ConstIterator::operator*() const {
  return *it_->second.get();
}

const RTCStats* RTCStatsReport::ConstIterator::operator->() const {
  return it_->second.get();
}

bool RTCStatsReport::ConstIterator::operator==(
    const RTCStatsReport::ConstIterator& other) const {
  return it_ == other.it_;
}

bool RTCStatsReport::ConstIterator::operator!=(
    const RTCStatsReport::ConstIterator& other) const {
  return !(*this == other);
}

void RTCStatsReport::AddStats(std::unique_ptr<const owt::base::RTCStats> stats) {
  stats_map.insert(std::make_pair(std::string(stats->id), std::move(stats)));
}

const RTCStats* RTCStatsReport::Get(const std::string& id) const {
  StatsMap::const_iterator it = stats_map.find(id);
  if (it != stats_map.cend())
    return it->second.get();
  return nullptr;
}

std::unique_ptr<const RTCStats> RTCStatsReport::Take(const std::string& id) {
  StatsMap::iterator it = stats_map.find(id);
  if (it == stats_map.end())
    return nullptr;
  std::unique_ptr<const RTCStats> stats = std::move(it->second);
  stats_map.erase(it);
  return stats;
}

void RTCStatsReport::TakeMembersFrom(
    std::shared_ptr<RTCStatsReport> victim) {
  for (StatsMap::iterator it = victim->stats_map.begin();
       it != victim->stats_map.end(); ++it) {
    AddStats(std::unique_ptr<const RTCStats>(it->second.release()));
  }
  victim->stats_map.clear();
}

RTCStatsReport::ConstIterator RTCStatsReport::begin() const {
  return ConstIterator((owt::base::RTCStatsReport*)(this),
                       stats_map.cbegin());
}

RTCStatsReport::ConstIterator RTCStatsReport::end() const {
  return ConstIterator((owt::base::RTCStatsReport*)(this),
                       stats_map.cend());
}

RTCCertificateStats::RTCCertificateStats(
    const std::string& id,
    int64_t timestamp,
    const std::string& fingerprint,
    const std::string& fingerprint_algorithm,
    const std::string& base64_certificate,
    const std::string& issuer_certificate_id)
    : RTCStats(RTCStatsType::kCertificate, id, timestamp),
      fingerprint(fingerprint),
      fingerprint_algorithm(fingerprint_algorithm),
      base64_certificate(base64_certificate),
      issuer_certificate_id(issuer_certificate_id) {}

RTCCertificateStats::RTCCertificateStats(const RTCCertificateStats& other)
    : RTCStats(other.type, other.id, other.timestamp_us),
      fingerprint(other.fingerprint),
      fingerprint_algorithm(other.fingerprint_algorithm),
      base64_certificate(other.base64_certificate),
      issuer_certificate_id(other.issuer_certificate_id) {}

RTCCertificateStats::~RTCCertificateStats() {}

RTCCodecStats::RTCCodecStats(const std::string& id,
                             int64_t timestamp,
                             uint32_t payload_type,
                             const std::string& mime_type,
                             uint32_t clock_rate,
                             uint32_t channels,
                             const std::string& sdp_fmtp_line)
     : RTCStats(RTCStatsType::kCodec, id, timestamp), payload_type(payload_type),
     mime_type(mime_type), clock_rate(clock_rate), sdp_fmtp_line(sdp_fmtp_line) {}


RTCCodecStats::RTCCodecStats(const RTCCodecStats& other)
    : RTCStats(other.type, other.id, other.timestamp_us),
      payload_type(other.payload_type),
      mime_type(other.mime_type),
      clock_rate(other.clock_rate),
      channels(other.channels),
      sdp_fmtp_line(other.sdp_fmtp_line) {}

RTCCodecStats::~RTCCodecStats() {}


RTCDataChannelStats::RTCDataChannelStats(const std::string& id,
                                         int64_t timestamp_us,
                                         const std::string& label,
                                         const std::string& protocol,
                                         int32_t data_channel_identifier,
                                         const std::string& state,
                                         uint32_t messages_sent,
                                         uint64_t bytes_sent,
                                         uint32_t messages_received,
                                         uint64_t bytes_received)
    : RTCStats(RTCStatsType::kDataChannel, id, timestamp_us),
      label(label),
      protocol(protocol),
      data_channel_identifier(data_channel_identifier),
      state(state),
      messages_sent(messages_sent),
      bytes_sent(bytes_sent),
      messages_received(messages_received),
      bytes_received(bytes_received) {}

RTCDataChannelStats::RTCDataChannelStats(const RTCDataChannelStats& other)
    : RTCStats(other.type, other.id, other.timestamp_us),
      label(other.label),
      protocol(other.protocol),
      data_channel_identifier(other.data_channel_identifier),
      state(other.state),
      messages_sent(other.messages_sent),
      bytes_sent(other.bytes_sent),
      messages_received(other.messages_received),
      bytes_received(other.bytes_received) {}

RTCDataChannelStats::~RTCDataChannelStats() {}


RTCIceCandidatePairStats::RTCIceCandidatePairStats(const std::string& id,
                           int64_t timestamp,
                           const std::string& transport_id,
                           const std::string& local_candidate_id,
                           const std::string& remote_candidate_id,
                           const std::string& state,
                           uint64_t priority,
                           bool nominated,
                           bool writable,
                           bool readable,
                           uint64_t bytes_sent,
                           uint64_t bytes_received,
                           double total_round_trip_time,
                           double current_round_trip_time,
                           double available_outgoing_bitrate,
                           double available_incoming_bitrate,
                           uint64_t requests_received,
                           uint64_t requests_sent,
                           uint64_t responses_received,
                           uint64_t responses_sent,
                           uint64_t retransmissions_received,
                           uint64_t retransmissions_sent,
                           uint64_t consent_requests_received,
                           uint64_t consent_requests_sent,
                           uint64_t consent_responses_received,
                           uint64_t consent_responses_sent)
    : RTCStats(RTCStatsType::kCandidatePair, id, timestamp),
      transport_id(transport_id),
      local_candidate_id(local_candidate_id),
      remote_candidate_id(remote_candidate_id),
      state(state),
      priority(priority),
      nominated(nominated),
      writable(writable),
      readable(readable),
      bytes_sent(bytes_sent),
      bytes_received(bytes_received),
      total_round_trip_time(total_round_trip_time),
      current_round_trip_time(current_round_trip_time),
      available_outgoing_bitrate(available_outgoing_bitrate),
      available_incoming_bitrate(available_incoming_bitrate),
      requests_received(requests_received),
      requests_sent(requests_sent),
      responses_received(responses_received),
      responses_sent(responses_sent),
      retransmissions_received(retransmissions_received),
      retransmissions_sent(retransmissions_sent),
      consent_requests_received(consent_requests_received),
      consent_requests_sent(consent_requests_sent),
      consent_responses_received(consent_responses_received),
      consent_responses_sent(consent_responses_sent) {}

RTCIceCandidatePairStats::RTCIceCandidatePairStats(
    const RTCIceCandidatePairStats& other)
    : RTCStats(other.type, other.id, other.timestamp_us),
      transport_id(other.transport_id),
      local_candidate_id(other.local_candidate_id),
      remote_candidate_id(other.remote_candidate_id),
      state(other.state),
      priority(other.priority),
      nominated(other.nominated),
      writable(other.writable),
      readable(other.readable),
      bytes_sent(other.bytes_sent),
      bytes_received(other.bytes_received),
      total_round_trip_time(other.total_round_trip_time),
      current_round_trip_time(other.current_round_trip_time),
      available_outgoing_bitrate(other.available_outgoing_bitrate),
      available_incoming_bitrate(other.available_incoming_bitrate),
      requests_received(other.requests_received),
      requests_sent(other.requests_sent),
      responses_received(other.responses_received),
      responses_sent(other.responses_sent),
      retransmissions_received(other.retransmissions_received),
      retransmissions_sent(other.retransmissions_sent),
      consent_requests_received(other.consent_requests_received),
      consent_requests_sent(other.consent_requests_sent),
      consent_responses_received(other.consent_responses_received),
      consent_responses_sent(other.consent_responses_sent) {}

RTCIceCandidatePairStats::~RTCIceCandidatePairStats() {}

RTCIceCandidateStats::RTCIceCandidateStats(const std::string& type,
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
                                           bool deleted)
    : RTCStats(type, id, timestamp),
      transport_id(transport_id),
      is_remote(is_remote),
      network_type(network_type),
      ip(ip),
      port(port),
      protocol(protocol),
      relay_protocol(relay_protocol),
      candidate_type(candidate_type),
      priority(priority),
      url(url),
      deleted(deleted) {}

RTCIceCandidateStats::RTCIceCandidateStats(const RTCIceCandidateStats& other)
    : RTCStats(other.type, other.id,  other.timestamp_us),
      transport_id(other.transport_id),
      is_remote(other.is_remote),
      network_type(other.network_type),
      ip(other.ip),
      port(other.port),
      protocol(other.protocol),
      relay_protocol(other.relay_protocol),
      candidate_type(other.candidate_type),
      priority(other.priority),
      url(other.url),
      deleted(other.deleted) {}

RTCIceCandidateStats::~RTCIceCandidateStats() {}

RTCLocalIceCandidateStats::RTCLocalIceCandidateStats(
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
    bool deleted)
    : RTCIceCandidateStats(RTCStatsType::kLocalCandidate, id, timestamp, transport_id, false,
        network_type, ip, port, protocol, relay_protocol, candidate_type,
        priority, url, deleted) {}

RTCLocalIceCandidateStats::RTCLocalIceCandidateStats(
    const RTCLocalIceCandidateStats& other)
    : RTCIceCandidateStats(other.type,
                           other.id,
                           other.timestamp_us,
                           other.transport_id,
                           other.is_remote,
                           other.network_type,
                           other.ip,
                           other.port,
                           other.protocol,
                           other.relay_protocol,
                           other.candidate_type,
                           other.priority,
                           other.url,
                           other.deleted) {}

RTCLocalIceCandidateStats::~RTCLocalIceCandidateStats() {}

RTCRemoteIceCandidateStats::RTCRemoteIceCandidateStats(
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
    bool deleted)
    : RTCIceCandidateStats(RTCStatsType::kRemoteCandidate,
                           id,
                           timestamp,
                           transport_id,
                           true,
                           network_type,
                           ip,
                           port,
                           protocol,
                           relay_protocol,
                           candidate_type,
                           priority,
                           url,
                           deleted) {}

RTCRemoteIceCandidateStats::RTCRemoteIceCandidateStats(
    const RTCRemoteIceCandidateStats& other)
    : RTCIceCandidateStats(other.type,
                           other.id,
                           other.timestamp_us,
                           other.transport_id,
                           other.is_remote,
                           other.network_type,
                           other.ip,
                           other.port,
                           other.protocol,
                           other.relay_protocol,
                           other.candidate_type,
                           other.priority,
                           other.url,
                           other.deleted) {}

RTCRemoteIceCandidateStats::~RTCRemoteIceCandidateStats() {}

RTCMediaStreamStats::RTCMediaStreamStats(
    const std::string& id,
    int64_t timestamp,
    const std::string& stream_identifier,
    const std::vector<std::string>& track_ids)
    : RTCStats(RTCStatsType::kStream, id, timestamp),
      stream_identifier(stream_identifier),
      track_ids(track_ids) {}

RTCMediaStreamStats::RTCMediaStreamStats(const RTCMediaStreamStats& other)
    : RTCStats(other.type, other.id, other.timestamp_us),
      stream_identifier(other.stream_identifier),
      track_ids(other.track_ids) {}

RTCMediaStreamStats::~RTCMediaStreamStats() {}

RTCMediaStreamTrackStats::RTCMediaStreamTrackStats(
    const std::string& id,
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
    double frames_per_second,
    uint32_t frames_sent,
    uint32_t huge_frames_sent,
    uint32_t frames_received,
    uint32_t frames_decoded,
    uint32_t frames_dropped,
    uint32_t frames_corrupted,
    uint32_t partial_frames_lost,
    uint32_t full_frames_lost,
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
    double jitter_buffer_target_delay,
    uint32_t interruption_count,
    double total_interruption_duration,
    uint32_t freeze_count,
    uint32_t pause_count,
    double total_freezes_duration,
    double total_pauses_duration,
    double total_frames_duration,
    double sum_squared_frame_durations)
      : RTCStats(RTCStatsType::kTrack, id, timestamp),
      track_identifier(track_identifier),
      media_source_id(media_source_id),
      remote_source(remote_source),
      ended(ended),
      detached(detached),
      kind(kind),
      jitter_buffer_delay(jitter_buffer_delay),
      jitter_buffer_emitted_count(jitter_buffer_emitted_count),
      frame_width(frame_width),
      frame_height(frame_height),
      frames_per_second(frames_per_second),
      frames_sent(frames_sent),
      huge_frames_sent(huge_frames_sent),
      frames_received(frames_received),
      frames_decoded(frames_decoded),
      frames_dropped(frames_dropped),
      frames_corrupted(frames_corrupted),
      partial_frames_lost(partial_frames_lost),
      full_frames_lost(full_frames_lost),
      audio_level(audio_level),
      total_audio_energy(total_audio_energy),
      echo_return_loss(echo_return_loss),
      echo_return_loss_enhancement(echo_return_loss_enhancement),
      total_samples_received(total_samples_received),
      total_samples_duration(total_samples_duration),
      concealed_samples(concealed_samples),
      silent_concealed_samples(silent_concealed_samples),
      concealment_events(concealment_events),
      inserted_samples_for_deceleration(
          inserted_samples_for_deceleration),
      removed_samples_for_acceleration(removed_samples_for_acceleration),
      jitter_buffer_flushes(jitter_buffer_flushes),
      delayed_packet_outage_samples(delayed_packet_outage_samples),
      relative_packet_arrival_delay(relative_packet_arrival_delay),
      jitter_buffer_target_delay(jitter_buffer_target_delay),
      interruption_count(interruption_count),
      total_interruption_duration(total_interruption_duration),
      freeze_count(freeze_count),
      pause_count(pause_count),
      total_freezes_duration(total_freezes_duration),
      total_pauses_duration(total_pauses_duration),
      total_frames_duration(total_frames_duration),
      sum_squared_frame_durations(sum_squared_frame_durations) {}

RTCMediaStreamTrackStats::RTCMediaStreamTrackStats(
    const RTCMediaStreamTrackStats& other)
    : RTCStats(other.type, other.id, other.timestamp_us),
      track_identifier(other.track_identifier),
      media_source_id(other.media_source_id),
      remote_source(other.remote_source),
      ended(other.ended),
      detached(other.detached),
      kind(other.kind),
      jitter_buffer_delay(other.jitter_buffer_delay),
      jitter_buffer_emitted_count(other.jitter_buffer_emitted_count),
      frame_width(other.frame_width),
      frame_height(other.frame_height),
      frames_per_second(other.frames_per_second),
      frames_sent(other.frames_sent),
      huge_frames_sent(other.huge_frames_sent),
      frames_received(other.frames_received),
      frames_decoded(other.frames_decoded),
      frames_dropped(other.frames_dropped),
      frames_corrupted(other.frames_corrupted),
      partial_frames_lost(other.partial_frames_lost),
      full_frames_lost(other.full_frames_lost),
      audio_level(other.audio_level),
      total_audio_energy(other.total_audio_energy),
      echo_return_loss(other.echo_return_loss),
      echo_return_loss_enhancement(other.echo_return_loss_enhancement),
      total_samples_received(other.total_samples_received),
      total_samples_duration(other.total_samples_duration),
      concealed_samples(other.concealed_samples),
      silent_concealed_samples(other.silent_concealed_samples),
      concealment_events(other.concealment_events),
      inserted_samples_for_deceleration(
          other.inserted_samples_for_deceleration),
      removed_samples_for_acceleration(other.removed_samples_for_acceleration),
      jitter_buffer_flushes(other.jitter_buffer_flushes),
      delayed_packet_outage_samples(other.delayed_packet_outage_samples),
      relative_packet_arrival_delay(other.relative_packet_arrival_delay),
      jitter_buffer_target_delay(other.jitter_buffer_target_delay),
      interruption_count(other.interruption_count),
      total_interruption_duration(other.total_interruption_duration),
      freeze_count(other.freeze_count),
      pause_count(other.pause_count),
      total_freezes_duration(other.total_freezes_duration),
      total_pauses_duration(other.total_pauses_duration),
      total_frames_duration(other.total_frames_duration),
      sum_squared_frame_durations(other.sum_squared_frame_durations) {}

RTCMediaStreamTrackStats::~RTCMediaStreamTrackStats() {}

RTCPeerConnectionStats::RTCPeerConnectionStats(const std::string& id,
                                               int64_t timestamp,
                                               uint32_t data_channels_opened,
                                               uint32_t data_channels_closed)
    : RTCStats(RTCStatsType::kPeerConnection, id, timestamp),
      data_channels_opened(data_channels_opened),
      data_channels_closed(data_channels_closed) {}

RTCPeerConnectionStats::RTCPeerConnectionStats(
    const RTCPeerConnectionStats& other)
    : RTCStats(other.type, other.id, other.timestamp_us),
      data_channels_opened(other.data_channels_opened),
      data_channels_closed(other.data_channels_closed) {}

RTCPeerConnectionStats::~RTCPeerConnectionStats() {}

RTCRTPStreamStats::RTCRTPStreamStats(const std::string& type,
                                     const std::string& id,
                                     int64_t timestamp,
                                     uint32_t ssrc,
                                     bool is_remote,
                                     const std::string& media_type,
                                     const std::string& kind,
                                     const std::string& track_id,
                                     const std::string& transport_id,
                                     const std::string& codec_id,
                                     uint32_t fir_count,
                                     uint32_t pli_count,
                                     uint32_t nack_count,
                                     uint32_t sli_count,
                                     uint64_t qp_sum)
    : RTCStats(type, id, timestamp),
      ssrc(ssrc),
      is_remote(is_remote),
      media_type(media_type),
      kind(kind),
      track_id(track_id),
      transport_id(transport_id),
      codec_id(codec_id),
      fir_count(fir_count),
      pli_count(pli_count),
      nack_count(nack_count),
      sli_count(sli_count),
      qp_sum(qp_sum) {}


RTCRTPStreamStats::RTCRTPStreamStats(const RTCRTPStreamStats& other)
    : RTCStats(other.type, other.id, other.timestamp_us),
      ssrc(other.ssrc),
      is_remote(other.is_remote),
      media_type(other.media_type),
      kind(other.kind),
      track_id(other.track_id),
      transport_id(other.transport_id),
      codec_id(other.codec_id),
      fir_count(other.fir_count),
      pli_count(other.pli_count),
      nack_count(other.nack_count),
      sli_count(other.sli_count),
      qp_sum(other.qp_sum) {}

RTCRTPStreamStats::~RTCRTPStreamStats() {}

RTCInboundRTPStreamStats::RTCInboundRTPStreamStats(
    const std::string& id,
    int64_t timestamp,
    uint32_t ssrc,
    bool is_remote,
    const std::string& media_type,
    const std::string& kind,
    const std::string& track_id,
    const std::string& transport_id,
    const std::string& codec_id,
    uint32_t fir_count,
    uint32_t pli_count,
    uint32_t nack_count,
    uint32_t sli_count,
    uint64_t qp_sum,
    uint32_t packets_received,
    uint64_t fec_packets_received,
    uint64_t fec_packets_discarded,
    uint64_t bytes_received,
    uint64_t header_bytes_received,
    int32_t packets_lost,
    double last_packet_received_timestamp,
    double jitter,
    double round_trip_time,
    uint32_t packets_discarded,
    uint32_t packets_repaired,
    uint32_t burst_packets_lost,
    uint32_t burst_packets_discarded,
    uint32_t burst_loss_count,
    uint32_t burst_discard_count,
    double burst_loss_rate,
    double burst_discard_rate,
    double gap_loss_rate,
    double gap_discard_rate,
    uint32_t frames_decoded,
    uint32_t frames_rendered,
    uint32_t key_frames_decoded,
    double total_decode_time,
    double total_inter_frame_delay,
    double total_squared_inter_frame_delay,
    const std::string& content_type,
    double estimated_playout_timestamp,
    const std::string& decoder_implementation)
    : RTCRTPStreamStats(RTCStatsType::kInboundRTP, id, timestamp, ssrc, is_remote,
         media_type, kind, track_id, transport_id, codec_id, fir_count, pli_count,
         nack_count, sli_count, qp_sum),
      packets_received(packets_received),
      fec_packets_received(fec_packets_received),
      fec_packets_discarded(fec_packets_discarded),
      bytes_received(bytes_received),
      header_bytes_received(header_bytes_received),
      packets_lost(packets_lost),
      last_packet_received_timestamp(last_packet_received_timestamp),
      jitter(jitter),
      round_trip_time(round_trip_time),
      packets_discarded(packets_discarded),
      packets_repaired(packets_repaired),
      burst_packets_lost(burst_packets_lost),
      burst_packets_discarded(burst_packets_discarded),
      burst_loss_count(burst_loss_count),
      burst_discard_count(burst_discard_count),
      burst_loss_rate(burst_loss_rate),
      burst_discard_rate(burst_discard_rate),
      gap_loss_rate(gap_loss_rate),
      gap_discard_rate(gap_discard_rate),
      frames_decoded(frames_decoded),
      frames_rendered(frames_rendered),
      key_frames_decoded(key_frames_decoded),
      total_decode_time(total_decode_time),
      total_inter_frame_delay(total_inter_frame_delay),
      total_squared_inter_frame_delay(total_squared_inter_frame_delay),
      content_type(content_type),
      estimated_playout_timestamp(estimated_playout_timestamp),
      decoder_implementation(decoder_implementation) {}

RTCInboundRTPStreamStats::RTCInboundRTPStreamStats(
    const RTCInboundRTPStreamStats& other)
    : RTCRTPStreamStats(other),
      packets_received(other.packets_received),
      fec_packets_received(other.fec_packets_received),
      fec_packets_discarded(other.fec_packets_discarded),
      bytes_received(other.bytes_received),
      header_bytes_received(other.header_bytes_received),
      packets_lost(other.packets_lost),
      last_packet_received_timestamp(other.last_packet_received_timestamp),
      jitter(other.jitter),
      round_trip_time(other.round_trip_time),
      packets_discarded(other.packets_discarded),
      packets_repaired(other.packets_repaired),
      burst_packets_lost(other.burst_packets_lost),
      burst_packets_discarded(other.burst_packets_discarded),
      burst_loss_count(other.burst_loss_count),
      burst_discard_count(other.burst_discard_count),
      burst_loss_rate(other.burst_loss_rate),
      burst_discard_rate(other.burst_discard_rate),
      gap_loss_rate(other.gap_loss_rate),
      gap_discard_rate(other.gap_discard_rate),
      frames_decoded(other.frames_decoded),
      frames_rendered(other.frames_rendered),
      key_frames_decoded(other.key_frames_decoded),
      total_decode_time(other.total_decode_time),
      total_inter_frame_delay(other.total_inter_frame_delay),
      total_squared_inter_frame_delay(other.total_squared_inter_frame_delay),
      content_type(other.content_type),
      estimated_playout_timestamp(other.estimated_playout_timestamp),
      decoder_implementation(other.decoder_implementation) {}

RTCInboundRTPStreamStats::~RTCInboundRTPStreamStats() {}

RTCOutboundRTPStreamStats::RTCOutboundRTPStreamStats(
    const std::string& id,
    int64_t timestamp,
    uint32_t ssrc,
    bool is_remote,
    const std::string& media_type,
    const std::string& kind,
    const std::string& track_id,
    const std::string& transport_id,
    const std::string& codec_id,
    uint32_t fir_count,
    uint32_t pli_count,
    uint32_t nack_count,
    uint32_t sli_count,
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
    const std::string& encoder_implementation)
      :RTCRTPStreamStats(RTCStatsType::kOutboundRTP, id, timestamp, ssrc,
         is_remote, media_type, kind, track_id, transport_id, codec_id,
         fir_count, pli_count, nack_count, sli_count, qp_sum),
      media_source_id(media_source_id),
      remote_id(remote_id),
      packets_sent(packets_sent),
      retransmitted_packets_sent(retransmitted_packets_sent),
      bytes_sent(bytes_sent),
      header_bytes_sent(header_bytes_sent),
      retransmitted_bytes_sent(retransmitted_bytes_sent),
      target_bitrate(target_bitrate),
      frames_encoded(frames_encoded),
      key_frames_encoded(key_frames_encoded),
      total_encode_time(total_encode_time),
      total_encoded_bytes_target(total_encoded_bytes_target),
      total_packet_send_delay(total_packet_send_delay),
      quality_limitation_reason(quality_limitation_reason),
      quality_limitation_resolution_changes(
          quality_limitation_resolution_changes),
      content_type(content_type),
      encoder_implementation(encoder_implementation) {}

RTCOutboundRTPStreamStats::RTCOutboundRTPStreamStats(
    const RTCOutboundRTPStreamStats& other)
    : RTCRTPStreamStats(other),
      media_source_id(other.media_source_id),
      remote_id(other.remote_id),
      packets_sent(other.packets_sent),
      retransmitted_packets_sent(other.retransmitted_packets_sent),
      bytes_sent(other.bytes_sent),
      header_bytes_sent(other.header_bytes_sent),
      retransmitted_bytes_sent(other.retransmitted_bytes_sent),
      target_bitrate(other.target_bitrate),
      frames_encoded(other.frames_encoded),
      key_frames_encoded(other.key_frames_encoded),
      total_encode_time(other.total_encode_time),
      total_encoded_bytes_target(other.total_encoded_bytes_target),
      total_packet_send_delay(other.total_packet_send_delay),
      quality_limitation_reason(other.quality_limitation_reason),
      quality_limitation_resolution_changes(
          other.quality_limitation_resolution_changes),
      content_type(other.content_type),
      encoder_implementation(other.encoder_implementation) {}

RTCOutboundRTPStreamStats::~RTCOutboundRTPStreamStats() {}

RTCRemoteInboundRtpStreamStats::RTCRemoteInboundRtpStreamStats(
    const std::string& id,
    int64_t timestamp,
    uint32_t ssrc,
    const std::string& kind,
    const std::string& transport_id,
    const std::string& codec_id,
    int32_t packets_lost,
    double jitter,
    const std::string& local_id,
    double round_trip_time)
    : RTCStats("remote-inbound-rtp", id, timestamp),
      ssrc(ssrc),
      kind(kind),
      transport_id(transport_id),
      codec_id(codec_id),
      packets_lost(packets_lost),
      jitter(jitter),
      local_id(local_id),
      round_trip_time(round_trip_time) {}

RTCRemoteInboundRtpStreamStats::RTCRemoteInboundRtpStreamStats(
    const RTCRemoteInboundRtpStreamStats& other)
    : RTCStats(other),
      ssrc(other.ssrc),
      kind(other.kind),
      transport_id(other.transport_id),
      codec_id(other.codec_id),
      packets_lost(other.packets_lost),
      jitter(other.jitter),
      local_id(other.local_id),
      round_trip_time(other.round_trip_time) {}

RTCRemoteInboundRtpStreamStats::~RTCRemoteInboundRtpStreamStats() {}

RTCMediaSourceStats::RTCMediaSourceStats(const std::string& type,
                                         const std::string& id,
                                         int64_t timestamp,
                                         const std::string& track_identifier,
                                         const std::string& kind)
    : RTCStats(type, id, timestamp),
      track_identifier(track_identifier),
      kind(kind) {}

RTCMediaSourceStats::RTCMediaSourceStats(const RTCMediaSourceStats& other)
    : RTCStats(other.type, other.id, other.timestamp_us),
      track_identifier(other.track_identifier),
      kind(other.kind) {}

RTCMediaSourceStats::~RTCMediaSourceStats() {}

RTCAudioSourceStats::RTCAudioSourceStats(const std::string& id,
                                         int64_t timestamp,
                                         const std::string& track_identifier,
                                         const std::string& kind,
                                         double audio_level,
                                         double total_audio_energy,
                                         double total_samples_duration)
    : RTCMediaSourceStats("media-source", id, timestamp, track_identifier, kind),
      audio_level(audio_level),
      total_audio_energy(total_audio_energy),
      total_samples_duration(total_samples_duration) {}

RTCAudioSourceStats::RTCAudioSourceStats(const RTCAudioSourceStats& other)
    : RTCMediaSourceStats(other),
      audio_level(other.audio_level),
      total_audio_energy(other.total_audio_energy),
      total_samples_duration(other.total_samples_duration) {}

RTCAudioSourceStats::~RTCAudioSourceStats() {}

RTCVideoSourceStats::RTCVideoSourceStats(const std::string& id,
                                         int64_t timestamp,
                                         const std::string& track_identifier,
                                         const std::string& kind,
                                         uint32_t width,
                                         uint32_t height,
                                         uint32_t frames,
                                         uint32_t frames_per_second)
    : RTCMediaSourceStats("media-source",
                          id,
                          timestamp,
                          track_identifier,
                          kind),
      width(width),
      height(height),
      frames(frames),
      frames_per_second(frames_per_second) {}

RTCVideoSourceStats::RTCVideoSourceStats(const RTCVideoSourceStats& other)
    : RTCMediaSourceStats(other),
      width(other.width),
      height(other.height),
      frames(other.frames),
      frames_per_second(other.frames_per_second) {}

RTCVideoSourceStats::~RTCVideoSourceStats() {}

RTCTransportStats::RTCTransportStats(
    const std::string& id,
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
    uint32_t selected_candidate_pair_changes)
    : RTCStats("transport", id, timestamp),
      bytes_sent(bytes_sent),
      bytes_received(bytes_received),
      rtcp_transport_stats_id(rtcp_transport_stats_id),
      dtls_state(dtls_state),
      selected_candidate_pair_id(selected_candidate_pair_id),
      local_certificate_id(local_certificate_id),
      remote_certificate_id(remote_certificate_id),
      tls_version(tls_version),
      dtls_cipher(dtls_cipher),
      srtp_cipher(srtp_cipher),
      selected_candidate_pair_changes(selected_candidate_pair_changes) {}

RTCTransportStats::RTCTransportStats(const RTCTransportStats& other)
    : RTCStats(other.type, other.id, other.timestamp_us),
      bytes_sent(other.bytes_sent),
      bytes_received(other.bytes_received),
      rtcp_transport_stats_id(other.rtcp_transport_stats_id),
      dtls_state(other.dtls_state),
      selected_candidate_pair_id(other.selected_candidate_pair_id),
      local_certificate_id(other.local_certificate_id),
      remote_certificate_id(other.remote_certificate_id),
      tls_version(other.tls_version),
      dtls_cipher(other.dtls_cipher),
      srtp_cipher(other.srtp_cipher),
      selected_candidate_pair_changes(other.selected_candidate_pair_changes) {}

RTCTransportStats::~RTCTransportStats() {}

}  // namespace base
}  // namespace owt