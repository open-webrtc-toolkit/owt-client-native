// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "OWT/OWTRemoteStream.h"
NS_ASSUME_NONNULL_BEGIN
@class OWTRemoteMixedStream;
RTC_OBJC_EXPORT
@protocol OWTRemoteMixedStreamDelegate<OWTRemoteStreamDelegate>
/**
  @brief Triggers when video layout is changed.
*/
- (void)streamDidChangeVideoLayout:(OWTRemoteMixedStream*)stream;
/**
  @brief Triggers when active speaker is changed.
*/
- (void)streamDidChangeActiveInput:(NSString*)activeAudioInputStreamId;
@end
/// This class represent a mixed remote stream.
RTC_OBJC_EXPORT
@interface OWTRemoteMixedStream : OWTRemoteStream
/**
  @brief A property of mixed streams which distinguishes them from other mixed
  streams a conference room provides.
  @details A conference room, since Intel CS for WebRTC v3.4 and later, has been
  extended to support multiple presentations of the mixed audio and video for
  variant purposes. For example, in remote education scenario, the teacher and
  students may subscribe different mixed streams with view of 'teacher' and
  'student' respectively in the same class conference room. It is also the label
  of a mixed stream indicating its peculiarity with a meaningful string-typed
  value, which must be unique within a room.
*/
@property(readonly, strong) NSString* viewport;
@property(nonatomic, weak) id<OWTRemoteMixedStreamDelegate> delegateMix;
@end
NS_ASSUME_NONNULL_END
