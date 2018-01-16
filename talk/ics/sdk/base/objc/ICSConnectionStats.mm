/*
 * Intel License
 */

#import "talk/ics/sdk/base/objc/ICSConnectionStats+Internal.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"

#include <unordered_map>
#include "webrtc/rtc_base/checks.h"

@implementation ICSConnectionStats {
  std::unordered_map<std::string, ICSIceCandidateStats*> _localCandidateMap;
  std::unordered_map<std::string, ICSIceCandidateStats*> _remoteCandidateMap;
}

- (instancetype)initWithNativeStats:
    (const ics::base::ConnectionStats&)stats {
  if (self = [super init]) {
    _timeStamp = [NSDate date];
    _videoBandwidthStats = [[ICSVideoBandwidthStats alloc]
        initWithNativeStats:stats.video_bandwidth_stats];

    NSMutableArray* mediaChannelStats = [[NSMutableArray alloc] init];
    for (auto& audio_sender : stats.audio_sender_reports) {
      [mediaChannelStats
          addObject:[[ICSAudioSenderStats alloc]
                        initWithNativeStats:*audio_sender.get()]];
    }
    for (auto& audio_receiver : stats.audio_receiver_reports) {
      [mediaChannelStats
          addObject:[[ICSAudioReceiverStats alloc]
                        initWithNativeStats:*audio_receiver.get()]];
    }
    for (auto& video_sender : stats.video_sender_reports) {
      [mediaChannelStats
          addObject:[[ICSVideoSenderStats alloc]
                        initWithNativeStats:*video_sender.get()]];
    }
    for (auto& video_receiver : stats.video_receiver_reports) {
      [mediaChannelStats
          addObject:[[ICSVideoReceiverStats alloc]
                        initWithNativeStats:*video_receiver.get()]];
    }
    _mediaChannelStats = mediaChannelStats;

    NSMutableArray<ICSIceCandidateStats*>* localCandidateStatsArray =
        [[NSMutableArray alloc] init];
    for (auto& local_candidate : stats.local_ice_candidate_reports) {
      ICSIceCandidateStats* localIceCandidate = [[ICSIceCandidateStats alloc]
          initWithNativeStats:*local_candidate.get()];
      [localCandidateStatsArray addObject:localIceCandidate];
      _localCandidateMap[local_candidate->id] = localIceCandidate;
    }
    _localIceCandidateStats = localCandidateStatsArray;

    NSMutableArray<ICSIceCandidateStats*>* remoteCandidateStatsArray =
        [[NSMutableArray alloc] init];
    for (auto& remote_candidate : stats.remote_ice_candidate_reports) {
      ICSIceCandidateStats* remoteIceCandidate = [[ICSIceCandidateStats alloc]
          initWithNativeStats:*remote_candidate.get()];
      [remoteCandidateStatsArray addObject:remoteIceCandidate];
      _remoteCandidateMap[remote_candidate->id] = remoteIceCandidate;
    }
    _remoteIceCandidateStats = remoteCandidateStatsArray;

    NSMutableArray<ICSIceCandidatePairStats*>* candidatePairArray =
        [[NSMutableArray alloc] init];
    for (auto& candidate_pair : stats.ice_candidate_pair_reports) {
      [candidatePairArray
          addObject:[[ICSIceCandidatePairStats alloc]
                        initWithNativeStats:*candidate_pair.get()
                          localIceCandidate:_localCandidateMap[candidate_pair ->local_ice_candidate->id]
                         remoteIceCandidate:_remoteCandidateMap[candidate_pair->remote_ice_candidate->id]]];
    }
    _iceCandidatePairStats = candidatePairArray;
  }
  return self;
}

@end

@implementation ICSMediaChannelStats

- (instancetype)initWithSsrc:(const std::string&)ssrc
                   codecName:(const std::string&)codecName
             trackIdentifier:(const std::string&)trackIdentifier {
  if (self = [super init]) {
    _ssrc = [NSString stringForStdString:ssrc];
    _codecName = [NSString stringForStdString:codecName];
    _trackIdentifier = [NSString stringForStdString:trackIdentifier];
  }
  return self;
}

@end

@implementation ICSAudioSenderStats

- (instancetype)initWithNativeStats:
    (const ics::base::AudioSenderReport&)stats {
  if (self = [super initWithSsrc:""
                       codecName:stats.codec_name
                 trackIdentifier:""]) {
    _bytesSent = (NSUInteger)stats.bytes_sent;
    _packetsSent = (NSUInteger)stats.packets_sent;
    _packetsLost = (NSUInteger)stats.packets_lost;
    _roundTripTime = (NSUInteger)stats.round_trip_time;
  }
  return self;
}

@end

@implementation ICSAudioReceiverStats

- (instancetype)initWithNativeStats:
    (const ics::base::AudioReceiverReport&)stats {
  if (self = [super initWithSsrc:""
                       codecName:stats.codec_name
                 trackIdentifier:""]) {
    _bytesReceived = (NSUInteger)stats.bytes_rcvd;
    _packetsReceived = (NSUInteger)stats.packets_rcvd;
    _packetsLost = (NSUInteger)stats.packets_lost;
    _estimatedDelay = (NSUInteger)stats.estimated_delay;
  }
  return self;
}

@end

@implementation ICSVideoSenderStats

- (instancetype)initWithNativeStats:
    (const ics::base::VideoSenderReport&)stats {
  if (self = [super initWithSsrc:""
                       codecName:stats.codec_name
                 trackIdentifier:""]) {
    _bytesSent = (NSUInteger)stats.bytes_sent;
    _packetsSent = (NSUInteger)stats.packets_sent;
    _packetsLost = (NSUInteger)stats.packets_lost;
    _roundTripTime = (NSUInteger)stats.round_trip_time;
    _firCount = (NSUInteger)stats.fir_count;
    _pliCount = (NSUInteger)stats.pli_count;
    _nackCount = (NSUInteger)stats.nack_count;
    _frameResolution = CGSizeMake(stats.frame_resolution_sent.width,
                                  stats.frame_resolution_sent.height);
    _frameRate = (NSUInteger)stats.framerate_sent;
    _adaptChanges = (NSUInteger)stats.adapt_changes;
  }
  return self;
}

@end

@implementation ICSVideoReceiverStats

- (instancetype)initWithNativeStats:
    (const ics::base::VideoReceiverReport&)stats {
  if (self = [super initWithSsrc:""
                       codecName:stats.codec_name
                 trackIdentifier:""]) {
    _bytesReceived = (NSUInteger)stats.bytes_rcvd;
    _packetsReceived = (NSUInteger)stats.packets_rcvd;
    _packetsLost = (NSUInteger)stats.packets_lost;
    _firCount = (NSUInteger)stats.fir_count;
    _pliCount = (NSUInteger)stats.pli_count;
    _nackCount = (NSUInteger)stats.nack_count;
    _frameResolution = CGSizeMake(stats.frame_resolution_rcvd.width,
                                  stats.frame_resolution_rcvd.height);
    _frameRateReceived = stats.framerate_rcvd;
    _frameRateOutput = stats.framerate_output;
    _delay = stats.delay;
  }
  return self;
}

@end

@implementation ICSVideoBandwidthStats

- (instancetype)initWithNativeStats:
    (const ics::base::VideoBandwidthStats&)stats {
  if (self = [super init]) {
    _availableSendBandwidth = (NSUInteger)stats.available_send_bandwidth;
    _availableReceiveBandwidth = (NSUInteger)stats.available_receive_bandwidth;
    _transmitBitrate = (NSUInteger)stats.transmit_bitrate;
    _retransmitBitrate = (NSUInteger)stats.retransmit_bitrate;
    _targetEncodingBitrate = (NSUInteger)stats.target_encoding_bitrate;
    _actualEncodingBitrate = (NSUInteger)stats.actual_encoding_bitrate;
  }
  return self;
}

@end


@implementation ICSIceCandidateStats

- (RTCIceCandidateType)getCandidateType:(ics::base::IceCandidateType)type {
  switch (type) {
    case ics::base::IceCandidateType::kHost:
      return RTCIceCandidateTypeHost;
    case ics::base::IceCandidateType::kSrflx:
      return RTCIceCandidateTypeSrflx;
    case ics::base::IceCandidateType::kPrflx:
      return RTCIceCandidateTypePrflx;
    case ics::base::IceCandidateType::kRelay:
      return RTCIceCandidateTypeRelay;
    default:
      RTC_DCHECK(false);
      return RTCIceCandidateTypeUnknown;
  }
}

- (RTCTransportProtocolType)getProtocolType:
    (ics::base::TransportProtocolType)type {
  switch (type) {
    case ics::base::TransportProtocolType::kTcp:
      return RTCTransportProtocolTypeTcp;
    case ics::base::TransportProtocolType::kUdp:
      return RTCTransportProtocolTypeUdp;
    default:
      RTC_DCHECK(false);
      return RTCTransportProtocolTypeUnknown;
  }
}

- (instancetype)initWithNativeStats:
    (const ics::base::IceCandidateReport&)stats {
  if (self = [super init]) {
    _statsId = [NSString stringForStdString:stats.id];
    _ip = [NSString stringForStdString:stats.ip];
    _port = (NSUInteger)stats.port;
    _candidateType = [self getCandidateType:stats.candidate_type];
    _protocol = [self getProtocolType:stats.protocol];
    _priority = (NSUInteger)stats.priority;
  }
  return self;
}

@end

@implementation ICSIceCandidatePairStats
- (instancetype)initWithNativeStats:
                    (const ics::base::IceCandidatePairReport&)stats
                  localIceCandidate:(ICSIceCandidateStats*)localIceCandidate
                 remoteIceCandidate:(ICSIceCandidateStats*)remoteIceCandidate {
  if (self = [super init]) {
    _statsId = [NSString stringForStdString:stats.id];
    _isActive = stats.is_active;
    _localIceCandidate = localIceCandidate;
    _remoteIceCandidate = remoteIceCandidate;
  }
  return self;
}
@end
