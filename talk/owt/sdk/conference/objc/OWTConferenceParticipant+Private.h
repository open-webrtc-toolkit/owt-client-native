// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "talk/owt/sdk/include/objc/OWT/OWTConferenceParticipant.h"
#include "talk/owt/sdk/include/cpp/owt/conference/conferenceclient.h"
/// This class represent an attendee in a conference.
RTC_OBJC_EXPORT
@interface OWTConferenceParticipant ()
@property(nonatomic, readonly)
    std::shared_ptr<owt::conference::Participant>
        nativeParticipant;
- (instancetype)initWithNativeParticipant:
    (std::shared_ptr<owt::conference::Participant>)participant;
@end
