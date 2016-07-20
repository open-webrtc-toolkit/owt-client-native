/*
 * Intel License
 */

#import "talk/woogeen/sdk/base/objc/RTCConnectionStats+Internal.h"
#import "webrtc/sdk/objc/Framework/Classes/NSString+StdString.h"

@implementation RTCConnectionStats

- (instancetype)initWithNativeStats:
    (const woogeen::base::ConnectionStats&)stats {
  if (self = [super init]) {
    _timeStamp = [NSDate date];
    _videoBandwidthStats = [[RTCVideoBandwidthStats alloc]
        initWithNativeStats:stats.video_bandwidth_stats];
    NSMutableArray* mediaChannelStats = [[NSMutableArray alloc] init];
    for (auto& audio_sender : stats.audio_sender_reports) {
      [mediaChannelStats
          addObject:[[RTCAudioSenderStats alloc]
                        initWithNativeStats:*audio_sender.get()]];
    }
    for (auto& audio_receiver : stats.audio_receiver_reports) {
      [mediaChannelStats
          addObject:[[RTCAudioReceiverStats alloc]
                        initWithNativeStats:*audio_receiver.get()]];
    }
    for (auto& video_sender : stats.video_sender_reports) {
      [mediaChannelStats
          addObject:[[RTCVideoSenderStats alloc]
                        initWithNativeStats:*video_sender.get()]];
    }
    for (auto& video_receiver : stats.video_receiver_reports) {
      [mediaChannelStats
          addObject:[[RTCVideoReceiverStats alloc]
                        initWithNativeStats:*video_receiver.get()]];
    }
    _mediaChannelStats = mediaChannelStats;
  }
  return self;
}

@end

@implementation RTCMediaChannelStats

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

@implementation RTCAudioSenderStats

- (instancetype)initWithNativeStats:
    (const woogeen::base::AudioSenderReport&)stats {
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

@implementation RTCAudioReceiverStats

- (instancetype)initWithNativeStats:
    (const woogeen::base::AudioReceiverReport&)stats {
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

@implementation RTCVideoSenderStats

- (instancetype)initWithNativeStats:
    (const woogeen::base::VideoSenderReport&)stats {
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

@implementation RTCVideoReceiverStats

- (instancetype)initWithNativeStats:
    (const woogeen::base::VideoReceiverReport&)stats {
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

@implementation RTCVideoBandwidthStats

- (instancetype)initWithNativeStats:
    (const woogeen::base::VideoBandwidthStats&)stats {
  if (self = [super init]) {
    _availableSendBandwidth = (NSUInteger)stats.available_send_bandwidth;
    _availableReceiveBandwidth = (NSUInteger)stats.available_receive_bandwidth;
    _transmitBitrate = (NSUInteger)stats.transmit_bitrate;
    _retransmitBitrate = (NSUInteger)stats.retransmit_bitrate;
  }
  return self;
}

@end
