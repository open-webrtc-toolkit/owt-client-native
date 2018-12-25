// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OMS_CONFERENCE_OBJC_OMSCONFERENCEPARTICIPANT_H_
#define OMS_CONFERENCE_OBJC_OMSCONFERENCEPARTICIPANT_H_
#import <Foundation/Foundation.h>
#import <WebRTC/RTCMacros.h>
/// This class represents an attendee in a conference.
RTC_EXPORT
@interface OMSConferenceParticipant : NSObject
- (instancetype)init NS_UNAVAILABLE;
/// The ID of the participant. It varies when a single user join different conferences.
@property(readonly, strong) NSString* participantId;
/// Role of the participant.
@property(readonly, strong) NSString* role;
/// The user ID of the participant. It can be integrated into existing account management system.
@property(readonly, strong) NSString* userId;
@end
#endif  // OMS_CONFERENCE_OBJC_OMSCONFERENCEPARTICIPANT_H_
