//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//
#import "talk/oms/sdk/include/objc/OMS/OMSMediaFormat.h"
#include "oms/base/commontypes.h"
#include "oms/base/options.h"
NS_ASSUME_NONNULL_BEGIN
// TODO: Change all native object to shared pointer so it can be changed in both
// C++ code and Objective-C code.
@interface OMSAudioCodecParameters ()
@property(nonatomic, readonly) std::shared_ptr<oms::base::AudioCodecParameters>
    nativeAudioCodecParameters;
- (instancetype)initWithNativeAudioCodecParameters:
    (const oms::base::AudioCodecParameters&)nativeAudioCodecParameters;
@end
@interface OMSVideoCodecParameters ()
@property(nonatomic, readonly) std::shared_ptr<oms::base::VideoCodecParameters>
    nativeVideoCodecParameters;
- (instancetype)initWithNativeVideoCodecParameters:
    (const oms::base::VideoCodecParameters &) nativeVideoCodecParameters;
@end
@interface OMSPublicationSettings ()
@property(nonatomic, readonly) oms::base::PublicationSettings nativeSettings;
- (instancetype)initWithNativePublicationSettings:
    (const oms::base::PublicationSettings &)nativeSettings;
@end

@interface OMSAudioPublicationSettings ()
@property(nonatomic, readonly) oms::base::AudioPublicationSettings nativeSettings;
- (instancetype)initWithNativeAudioPublicationSettings:
    (const oms::base::AudioPublicationSettings &)nativeSettings;
@end
@interface OMSVideoPublicationSettings ()
@property(nonatomic, readonly) oms::base::VideoPublicationSettings nativeSettings;
- (instancetype)initWithNativeVideoPublicationSettings:
    (const oms::base::VideoPublicationSettings &)nativeSettings;
@end
@interface OMSAudioSubscriptionCapabilities()
@property(nonatomic, readonly) oms::base::AudioSubscriptionCapabilities nativeCapabilities;
- (instancetype)initWithNativeAudioSubscriptionCapabilities:
    (const oms::base::AudioSubscriptionCapabilities &)nativeCapabilities;
@end
@interface OMSVideoSubscriptionCapabilities()
@property(nonatomic, readonly) oms::base::VideoSubscriptionCapabilities nativeCapabilities;
- (instancetype)initWithNativeVideoSubscriptionCapabilities:
    (const oms::base::VideoSubscriptionCapabilities &)nativeCapabilities;
@end
@interface OMSSubscriptionCapabilities()
@property(nonatomic, readonly) oms::base::SubscriptionCapabilities nativeCapabilities;
- (instancetype)initWithNativeSubscriptionCapabilities:
    (const oms::base::SubscriptionCapabilities &)nativeCapabilities;
@end
@interface OMSStreamSourceInfo ()
@property(nonatomic, readonly)
    oms::base::StreamSourceInfo nativeStreamSourceInfo;
- (instancetype)initWithNativeStreamSourceInfo:
    (std::shared_ptr<oms::base::StreamSourceInfo>)nativeSource;
@end
@interface OMSVideoTrackConstraints ()
@end
@interface OMSStreamConstraints ()
@end
@interface OMSAudioEncodingParameters ()
@property(nonatomic, readonly)
    oms::base::AudioEncodingParameters nativeAudioEncodingParameters;
@end
@interface OMSVideoEncodingParameters ()
@property(nonatomic, readonly)
    oms::base::VideoEncodingParameters nativeVideoEncodingParameters;
@end
@interface OMSTrackKindConverter : NSObject
+ (oms::base::TrackKind)cppTrackKindForObjcTrackKind:(OMSTrackKind)kind;
+ (OMSTrackKind)objcTrackKindForCppTrackKind:(oms::base::TrackKind)kind;
@end
NS_ASSUME_NONNULL_END
