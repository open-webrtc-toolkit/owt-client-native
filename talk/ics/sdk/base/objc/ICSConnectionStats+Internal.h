/*
 * Intel License
 */

#import "talk/ics/sdk/include/objc/ICS/ICSConnectionStats.h"

#include "talk/ics/sdk/include/cpp/ics/base/connectionstats.h"

@interface ICSConnectionStats (Internal)
- (instancetype)initWithNativeStats:
    (const ics::base::ConnectionStats&)stats;
@end

@interface ICSMediaChannelStats (Internal)
- (instancetype)initWithSsrc:(const std::string&)ssrc
                   codecName:(const std::string&)codecName
             trackIdentifier:(const std::string&)trackIdentifier;
@end

@interface ICSAudioSenderStats (Internal)
- (instancetype)initWithNativeStats:
    (const ics::base::AudioSenderReport&)stats;
@end

@interface ICSAudioReceiverStats (Internal)
- (instancetype)initWithNativeStats:
    (const ics::base::AudioReceiverReport&)stats;
@end

@interface ICSVideoSenderStats (Internal)
- (instancetype)initWithNativeStats:
    (const ics::base::VideoSenderReport&)stats;
@end

@interface ICSVideoReceiverStats (Internal)
- (instancetype)initWithNativeStats:
    (const ics::base::VideoReceiverReport&)stats;
@end

@interface ICSVideoBandwidthStats (Internal)
- (instancetype)initWithNativeStats:
    (const ics::base::VideoBandwidthStats&)stats;
@end

@interface ICSIceCandidateStats (Internal)
- (instancetype)initWithNativeStats:
    (const ics::base::IceCandidateReport&)stats;
@end

@interface ICSIceCandidatePairStats (Internal)
- (instancetype)initWithNativeStats:
                    (const ics::base::IceCandidatePairReport&)stats
                  localIceCandidate:(ICSIceCandidateStats*)localIceCandidate
                 remoteIceCandidate:(ICSIceCandidateStats*)remoteIceCandidate;
@end
