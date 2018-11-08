//
//  Copyright (c) 2018 Intel Corporation. All rights reserved.
//
#import "talk/oms/sdk/include/objc/OMS/OMSConferenceInfo.h"
#include "talk/oms/sdk/include/cpp/oms/conference/conferenceclient.h"
NS_ASSUME_NONNULL_BEGIN
RTC_EXPORT
@interface OMSConferenceInfo ()
@property(nonatomic, readonly) std::shared_ptr<const oms::conference::ConferenceInfo> nativeInfo;
- (instancetype)initWithNativeInfo:
    (std::shared_ptr<const oms::conference::ConferenceInfo>)info;
@end
NS_ASSUME_NONNULL_END
