/*
 * Intel License
 */

#import "talk/ics/sdk/include/objc/Woogeen/RTCConnectionStats.h"

#include "talk/ics/sdk/include/cpp/ics/base/connectionstats.h"

@interface RTCConnectionStats (Internal)
- (instancetype)initWithNativeStats:
    (const ics::base::ConnectionStats&)stats;
@end

@interface RTCMediaChannelStats (Internal)
- (instancetype)initWithSsrc:(const std::string&)ssrc
                   codecName:(const std::string&)codecName
             trackIdentifier:(const std::string&)trackIdentifier;
@end

@interface RTCAudioSenderStats (Internal)
- (instancetype)initWithNativeStats:
    (const ics::base::AudioSenderReport&)stats;
@end

@interface RTCAudioReceiverStats (Internal)
- (instancetype)initWithNativeStats:
    (const ics::base::AudioReceiverReport&)stats;
@end

@interface RTCVideoSenderStats (Internal)
- (instancetype)initWithNativeStats:
    (const ics::base::VideoSenderReport&)stats;
@end

@interface RTCVideoReceiverStats (Internal)
- (instancetype)initWithNativeStats:
    (const ics::base::VideoReceiverReport&)stats;
@end

@interface RTCVideoBandwidthStats (Internal)
- (instancetype)initWithNativeStats:
    (const ics::base::VideoBandwidthStats&)stats;
@end

@interface RTCIceCandidateStats (Internal)
- (instancetype)initWithNativeStats:
    (const ics::base::IceCandidateReport&)stats;
@end

@interface RTCIceCandidatePairStats (Internal)
- (instancetype)initWithNativeStats:
                    (const ics::base::IceCandidatePairReport&)stats
                  localIceCandidate:(RTCIceCandidateStats*)localIceCandidate
                 remoteIceCandidate:(RTCIceCandidateStats*)remoteIceCandidate;
@end
