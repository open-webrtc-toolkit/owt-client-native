//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/include/objc/Woogeen/RTCMediaFormat.h"

#include "talk/ics/sdk/include/cpp/ics/base/common_types.h"

#ifndef WOOGEEN_CONFERENCE_OBJC_RTCREMOTEMIXEDSTREAM_INTERNAL_H_
#define WOOGEEN_CONFERENCE_OBJC_RTCREMOTEMIXEDSTREAM_INTERNAL_H_

@interface RTCVideoFormat (Internal)

- (instancetype)initWithNativeVideoFormat:
    (const ics::base::VideoFormat&)videoFormat;

@end

#endif  // WOOGEEN_CONFERENCE_OBJC_RTCREMOTEMIXEDSTREAM_INTERNAL_H_
