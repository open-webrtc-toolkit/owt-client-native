//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#ifndef WOOGEEN_CONFERENCE_OBJC_RTCMEDIACODEC_INTERNAL_H_
#define WOOGEEN_CONFERENCE_OBJC_RTCMEDIACODEC_INTERNAL_H_

#import "talk/woogeen/sdk/base/objc/public/RTCMediaCodec.h"

#include "talk/woogeen/sdk/include/cpp/woogeen/base/mediaformat.h"

@interface RTCMediaCodec (Internal)

+ (woogeen::base::MediaCodec::AudioCodec)nativeAudioCodec:(NSInteger)audioCodec;
+ (woogeen::base::MediaCodec::VideoCodec)nativeVideoCodec:(NSInteger)videoCodec;

@end

#endif  // WOOGEEN_CONFERENCE_OBJC_RTCMEDIACODEC_INTERNAL_H_
