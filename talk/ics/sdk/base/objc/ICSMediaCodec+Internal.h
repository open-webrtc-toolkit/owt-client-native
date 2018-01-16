//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#ifndef ICS_CONFERENCE_OBJC_RTCMEDIACODEC_INTERNAL_H_
#define ICS_CONFERENCE_OBJC_RTCMEDIACODEC_INTERNAL_H_

#import "talk/ics/sdk/include/objc/ICS/ICSMediaCodec.h"

#include "talk/ics/sdk/include/cpp/ics/base/common_types.h"

@interface ICSMediaCodec (Internal)

+ (ics::base::AudioCodec)nativeAudioCodec:(NSInteger)audioCodec;
+ (ics::base::VideoCodec)nativeVideoCodec:(NSInteger)videoCodec;

@end

#endif  // ICS_CONFERENCE_OBJC_RTCMEDIACODEC_INTERNAL_H_
