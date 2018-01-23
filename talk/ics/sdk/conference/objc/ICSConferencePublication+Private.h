//
//  Copyright (c) 2018 Intel Corporation. All rights reserved.
//

#include <memory>
#include "talk/ics/sdk/include/cpp/ics/conference/conferencepublication.h"

#import "ICS/ICSConferencePublication.h"

NS_ASSUME_NONNULL_BEGIN

RTC_EXPORT
@interface ICSConferencePublication ()

- (instancetype)initWithNativePublication:
    (std::shared_ptr<ics::conference::ConferencePublication>)
        nativePublication;  // TODO: Moving to ics::base::Publication.

@end

NS_ASSUME_NONNULL_END
