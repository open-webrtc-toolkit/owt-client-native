//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/include/objc/ICS/ICSMediaFormat.h"

#include "ics/base/commontypes.h"
#include "ics/base/options.h"

@interface ICSAudioCodecParameters ()

@property(nonatomic, readonly)
    ics::base::AudioCodecParameters nativeAudioCodecParameters;

- (instancetype)initWithNativeAudioCodecParameters:
    (const ics::base::AudioCodecParameters &)nativeAudioCodecParameters;

@end

@interface ICSVideoCodecParameters ()

@property(nonatomic, readonly) ics::base::VideoCodecParameters nativeVideoCodecParameters;

- (instancetype)initWithNativeVideoCodecParameters:
    (const ics::base::VideoCodecParameters &) nativeVideoCodecParameters;

@end


@interface ICSVideoFormat ()

- (instancetype)initWithNativeVideoFormat:
    (const ics::base::VideoFormat&)videoFormat;

@end

@interface ICSPublicationSettings ()

@property(nonatomic, readonly) ics::base::PublicationSettings nativeSettings;

- (instancetype)initWithNativePublicationSettings:
    (const ics::base::PublicationSettings &)nativeSettings;

@end


@interface ICSAudioPublicationSettings ()

@property(nonatomic, readonly) ics::base::AudioPublicationSettings nativeSettings;

- (instancetype)initWithNativeAudioPublicationSettings:
    (const ics::base::AudioPublicationSettings &)nativeSettings;

@end

@interface ICSVideoPublicationSettings ()

@property(nonatomic, readonly) ics::base::VideoPublicationSettings nativeSettings;

- (instancetype)initWithNativeVideoPublicationSettings:
    (const ics::base::VideoPublicationSettings &)nativeSettings;

@end

@interface ICSAudioSubscriptionCapabilities()

@property(nonatomic, readonly) ics::base::AudioSubscriptionCapabilities nativeCapabilities;

- (instancetype)initWithNativeAudioSubscriptionCapabilities:
    (const ics::base::AudioSubscriptionCapabilities &)nativeCapabilities;

@end

@interface ICSVideoSubscriptionCapabilities()

@property(nonatomic, readonly) ics::base::VideoSubscriptionCapabilities nativeCapabilities;

- (instancetype)initWithNativeVideoSubscriptionCapabilities:
    (const ics::base::VideoSubscriptionCapabilities &)nativeCapabilities;

@end

@interface ICSSubscriptionCapabilities()

@property(nonatomic, readonly) ics::base::SubscriptionCapabilities nativeCapabilities;

- (instancetype)initWithNativeSubscriptionCapabilities:
    (const ics::base::SubscriptionCapabilities &)nativeCapabilities;

@end
