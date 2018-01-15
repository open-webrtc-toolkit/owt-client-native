//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#ifndef WOOGEEN_CONFERENCE_OBJC_RTCMEDIACODEC_INTERNAL_H_
#define WOOGEEN_CONFERENCE_OBJC_RTCMEDIACODEC_INTERNAL_H_

#import "talk/ics/sdk/include/objc/Woogeen/RTCMediaCodec.h"

#include "talk/ics/sdk/include/cpp/ics/base/common_types.h"

@interface RTCMediaCodec (Internal)

+ (ics::base::AudioCodec)nativeAudioCodec:(NSInteger)audioCodec;
+ (ics::base::VideoCodec)nativeVideoCodec:(NSInteger)videoCodec;

@end

#endif  // WOOGEEN_CONFERENCE_OBJC_RTCMEDIACODEC_INTERNAL_H_
