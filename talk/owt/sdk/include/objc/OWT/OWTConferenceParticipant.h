// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_CONFERENCE_OBJC_OWTCONFERENCEPARTICIPANT_H_
#define OWT_CONFERENCE_OBJC_OWTCONFERENCEPARTICIPANT_H_
#import <Foundation/Foundation.h>
#import <WebRTC/RTCMacros.h>
/// This class represents an attendee in a conference.
RTC_EXPORT
@interface OWTConferenceParticipant : NSObject
- (instancetype)init NS_UNAVAILABLE;
/// The ID of the participant. It varies when a single user join different conferences.
@property(readonly, strong) NSString* participantId;
/// Role of the participant.
@property(readonly, strong) NSString* role;
/// The user ID of the participant. It can be integrated into existing account management system.
@property(readonly, strong) NSString* userId;
@end
#endif  // OWT_CONFERENCE_OBJC_OWTCONFERENCEPARTICIPANT_H_
