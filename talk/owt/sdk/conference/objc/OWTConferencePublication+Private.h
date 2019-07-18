// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include <memory>
#include "talk/owt/sdk/include/cpp/owt/conference/conferencepublication.h"
#import "OWT/OWTConferencePublication.h"
NS_ASSUME_NONNULL_BEGIN
RTC_OBJC_EXPORT
@interface OWTConferencePublication ()
- (instancetype)initWithNativePublication:
    (std::shared_ptr<owt::conference::ConferencePublication>)
        nativePublication;  // TODO: Moving to owt::base::Publication.
@end
NS_ASSUME_NONNULL_END
