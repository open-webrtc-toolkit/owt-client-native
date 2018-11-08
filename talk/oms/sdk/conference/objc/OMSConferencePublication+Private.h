//
//  Copyright (c) 2018 Intel Corporation. All rights reserved.
//
#include <memory>
#include "talk/oms/sdk/include/cpp/oms/conference/conferencepublication.h"
#import "OMS/OMSConferencePublication.h"
NS_ASSUME_NONNULL_BEGIN
RTC_EXPORT
@interface OMSConferencePublication ()
- (instancetype)initWithNativePublication:
    (std::shared_ptr<oms::conference::ConferencePublication>)
        nativePublication;  // TODO: Moving to oms::base::Publication.
@end
NS_ASSUME_NONNULL_END
