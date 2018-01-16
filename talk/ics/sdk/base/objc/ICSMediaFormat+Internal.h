//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/include/objc/ICS/ICSMediaFormat.h"

#include "talk/ics/sdk/include/cpp/ics/base/common_types.h"

#ifndef ICS_CONFERENCE_OBJC_ICSRemoteMixedStream_INTERNAL_H_
#define ICS_CONFERENCE_OBJC_ICSRemoteMixedStream_INTERNAL_H_

@interface ICSVideoFormat (Internal)

- (instancetype)initWithNativeVideoFormat:
    (const ics::base::VideoFormat&)videoFormat;

@end

#endif  // ICS_CONFERENCE_OBJC_ICSRemoteMixedStream_INTERNAL_H_
