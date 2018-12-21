// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <WebRTC/RTCMacros.h>
#import <OMS/OMSConferenceParticipant.h>
#import <OMS/OMSRemoteStream.h>
NS_ASSUME_NONNULL_BEGIN
/**
  @brief Information about the conference.
  @details This information contains current details of the conference.
*/
RTC_EXPORT
@interface OMSConferenceInfo : NSObject
- (instancetype)init NS_UNAVAILABLE;
/// Conference ID
@property(readonly, strong) NSString* conferenceId;
/// Participants in the conference.
@property(readonly, strong) NSArray<OMSConferenceParticipant*>* participants;
/**
  @brief Streams published by participants.
  @details It also includes streams published by current user.
*/
@property(readonly, strong) NSArray<OMSRemoteStream*>* remoteStreams;
/// Current user's info.
@property(readonly, strong) OMSConferenceParticipant* myself;
@end
NS_ASSUME_NONNULL_END
