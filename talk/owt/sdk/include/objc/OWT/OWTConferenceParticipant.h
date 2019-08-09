// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_CONFERENCE_OBJC_OWTCONFERENCEPARTICIPANT_H_
#define OWT_CONFERENCE_OBJC_OWTCONFERENCEPARTICIPANT_H_
#import <Foundation/Foundation.h>
#import <WebRTC/RTCMacros.h>
NS_ASSUME_NONNULL_BEGIN
@protocol OWTConferenceParticipantDelegate;

/// This class represents an attendee in a conference.
RTC_OBJC_EXPORT
@interface OWTConferenceParticipant : NSObject
- (instancetype)init NS_UNAVAILABLE;
/// The ID of the participant. It varies when a single user join different conferences.
@property(readonly, strong) NSString* participantId;
/// Role of the participant.
@property(readonly, strong) NSString* role;
/// The user ID of the participant. It can be integrated into existing account management system.
@property(readonly, strong) NSString* userId;

@property(nonatomic, weak) id<OWTConferenceParticipantDelegate> delegate;
@end

RTC_OBJC_EXPORT
@protocol OWTConferenceParticipantDelegate<NSObject>
@optional
/// Participant leave conference.
- (void)participantDidLeave:(OWTConferenceParticipant*)participant;
@end
NS_ASSUME_NONNULL_END
#endif  // OWT_CONFERENCE_OBJC_OWTCONFERENCEPARTICIPANT_H_
