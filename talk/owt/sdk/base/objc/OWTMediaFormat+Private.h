// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "talk/owt/sdk/include/objc/OWT/OWTMediaFormat.h"
#include "owt/base/commontypes.h"
#include "owt/base/options.h"
NS_ASSUME_NONNULL_BEGIN
// TODO: Change all native object to shared pointer so it can be changed in both
// C++ code and Objective-C code.
@interface OWTAudioCodecParameters ()
@property(nonatomic, readonly) std::shared_ptr<owt::base::AudioCodecParameters>
    nativeAudioCodecParameters;
- (instancetype)initWithNativeAudioCodecParameters:
    (const owt::base::AudioCodecParameters&)nativeAudioCodecParameters;
@end
@interface OWTVideoCodecParameters ()
@property(nonatomic, readonly) std::shared_ptr<owt::base::VideoCodecParameters>
    nativeVideoCodecParameters;
- (instancetype)initWithNativeVideoCodecParameters:
    (const owt::base::VideoCodecParameters &) nativeVideoCodecParameters;
@end
@interface OWTPublicationSettings ()
@property(nonatomic, readonly) owt::base::PublicationSettings nativeSettings;
- (instancetype)initWithNativePublicationSettings:
    (const owt::base::PublicationSettings &)nativeSettings;
@end

@interface OWTAudioPublicationSettings ()
@property(nonatomic, readonly) owt::base::AudioPublicationSettings nativeSettings;
- (instancetype)initWithNativeAudioPublicationSettings:
    (const owt::base::AudioPublicationSettings &)nativeSettings;
@end
@interface OWTVideoPublicationSettings ()
@property(nonatomic, readonly) owt::base::VideoPublicationSettings nativeSettings;
- (instancetype)initWithNativeVideoPublicationSettings:
    (const owt::base::VideoPublicationSettings &)nativeSettings;
@end
@interface OWTAudioSubscriptionCapabilities()
@property(nonatomic, readonly) owt::base::AudioSubscriptionCapabilities nativeCapabilities;
- (instancetype)initWithNativeAudioSubscriptionCapabilities:
    (const owt::base::AudioSubscriptionCapabilities &)nativeCapabilities;
@end
@interface OWTVideoSubscriptionCapabilities()
@property(nonatomic, readonly) owt::base::VideoSubscriptionCapabilities nativeCapabilities;
- (instancetype)initWithNativeVideoSubscriptionCapabilities:
    (const owt::base::VideoSubscriptionCapabilities &)nativeCapabilities;
@end
@interface OWTSubscriptionCapabilities()
@property(nonatomic, readonly) owt::base::SubscriptionCapabilities nativeCapabilities;
- (instancetype)initWithNativeSubscriptionCapabilities:
    (const owt::base::SubscriptionCapabilities &)nativeCapabilities;
@end
@interface OWTStreamSourceInfo ()
@property(nonatomic, readonly)
    owt::base::StreamSourceInfo nativeStreamSourceInfo;
- (instancetype)initWithNativeStreamSourceInfo:
    (std::shared_ptr<owt::base::StreamSourceInfo>)nativeSource;
@end
@interface OWTVideoTrackConstraints ()
@end
@interface OWTStreamConstraints ()
@end
@interface OWTAudioEncodingParameters ()
@property(nonatomic, readonly)
    owt::base::AudioEncodingParameters nativeAudioEncodingParameters;
@end
@interface OWTVideoEncodingParameters ()
@property(nonatomic, readonly)
    owt::base::VideoEncodingParameters nativeVideoEncodingParameters;
@end
@interface OWTTrackKindConverter : NSObject
+ (owt::base::TrackKind)cppTrackKindForObjcTrackKind:(OWTTrackKind)kind;
+ (OWTTrackKind)objcTrackKindForCppTrackKind:(owt::base::TrackKind)kind;
@end
NS_ASSUME_NONNULL_END
