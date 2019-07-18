// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "talk/owt/sdk/include/objc/OWT/OWTConferenceInfo.h"
#include "talk/owt/sdk/include/cpp/owt/conference/conferenceclient.h"
NS_ASSUME_NONNULL_BEGIN
RTC_OBJC_EXPORT
@interface OWTConferenceInfo ()
@property(nonatomic, readonly) std::shared_ptr<const owt::conference::ConferenceInfo> nativeInfo;
- (instancetype)initWithNativeInfo:
    (std::shared_ptr<const owt::conference::ConferenceInfo>)info;
@end
NS_ASSUME_NONNULL_END
