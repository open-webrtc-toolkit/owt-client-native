/*
 * Intel License
 */

#import "talk/woogeen/sdk/include/objc/Woogeen/RTCConnectionStats.h"

#include "talk/woogeen/sdk/include/cpp/woogeen/base/connectionstats.h"

@interface RTCConnectionStats (Internal)
- (instancetype)initWithNativeStats:
    (const woogeen::base::ConnectionStats&)stats;
@end

@interface RTCMediaChannelStats (Internal)
- (instancetype)initWithSsrc:(const std::string&)ssrc
                   codecName:(const std::string&)codecName
             trackIdentifier:(const std::string&)trackIdentifier;
@end

@interface RTCAudioSenderStats (Internal)
- (instancetype)initWithNativeStats:
    (const woogeen::base::AudioSenderReport&)stats;
@end

@interface RTCAudioReceiverStats (Internal)
- (instancetype)initWithNativeStats:
    (const woogeen::base::AudioReceiverReport&)stats;
@end

@interface RTCVideoSenderStats (Internal)
- (instancetype)initWithNativeStats:
    (const woogeen::base::VideoSenderReport&)stats;
@end

@interface RTCVideoReceiverStats (Internal)
- (instancetype)initWithNativeStats:
    (const woogeen::base::VideoReceiverReport&)stats;
@end

@interface RTCVideoBandwidthStats (Internal)
- (instancetype)initWithNativeStats:
    (const woogeen::base::VideoBandwidthStats&)stats;
@end

@interface RTCIceCandidateStats (Internal)
- (instancetype)initWithNativeStats:
    (const woogeen::base::IceCandidateReport&)stats;
@end

@interface RTCIceCandidatePairStats (Internal)
- (instancetype)initWithNativeStats:
                    (const woogeen::base::IceCandidatePairReport&)stats
                  localIceCandidate:(RTCIceCandidateStats*)localIceCandidate
                 remoteIceCandidate:(RTCIceCandidateStats*)remoteIceCandidate;
@end
