//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/include/objc/ICS/ICSConferenceParticipant.h"

#include "talk/ics/sdk/include/cpp/ics/conference/conferenceclient.h"

/// This class represent an attendee in a conference.
RTC_EXPORT
@interface ICSConferenceParticipant ()

@property(nonatomic, readonly)
    std::shared_ptr<const ics::conference::Participant>
        nativeParticipant;

- (instancetype)initWithNativeParticipant:
    (std::shared_ptr<const ics::conference::Participant>)participant;

@end
