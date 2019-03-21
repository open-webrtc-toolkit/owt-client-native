// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "talk/owt/sdk/include/cpp/owt/base/commontypes.h"
#import "talk/owt/sdk/base/objc/OWTRemoteStream+Private.h"
#import "talk/owt/sdk/conference/objc/OWTConferenceInfo+Private.h"
#import "talk/owt/sdk/conference/objc/OWTConferenceParticipant+Private.h"
#import "talk/owt/sdk/include/objc/OWT/OWTRemoteMixedStream.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
@implementation OWTConferenceInfo
@dynamic conferenceId, participants, remoteStreams, myself;
- (instancetype)initWithNativeInfo:
    (std::shared_ptr<const owt::conference::ConferenceInfo>)info {
  self = [super init];
  _nativeInfo = info;
  return self;
}
- (NSString*)conferenceId {
  return [NSString stringForStdString:_nativeInfo->Id()];
}
- (NSArray<OWTConferenceParticipant*>*)participants {
  const auto& nativeParticipants = _nativeInfo->Participants();
  NSMutableArray<OWTConferenceParticipant*>* participants =
      [NSMutableArray arrayWithCapacity:nativeParticipants.size()];
  for (const auto& nativeParticipant : nativeParticipants) {
    OWTConferenceParticipant* participant = [[OWTConferenceParticipant alloc]
        initWithNativeParticipant:nativeParticipant];
    [participants addObject:participant];
  }
  return participants;
}
- (NSArray<OWTRemoteStream*>*)remoteStreams {
  const auto& nativeStreams = _nativeInfo->RemoteStreams();
  NSMutableArray<OWTRemoteStream*>* streams =
      [NSMutableArray arrayWithCapacity:nativeStreams.size()];
  for (const auto& nativeStream : nativeStreams) {
    if (nativeStream->Source().audio == owt::base::AudioSourceInfo::kMixed &&
        nativeStream->Source().video == owt::base::VideoSourceInfo::kMixed) {
      OWTRemoteMixedStream* stream =
          [[OWTRemoteMixedStream alloc] initWithNativeStream:nativeStream];
      [streams addObject:stream];
    } else {
      OWTRemoteStream* stream =
          [[OWTRemoteStream alloc] initWithNativeStream:nativeStream];
      [streams addObject:stream];
    }
  }
  return streams;
}
- (OWTConferenceParticipant*)myself {
  const auto& nativeParticipant = _nativeInfo->Self();
  OWTConferenceParticipant* participant = [[OWTConferenceParticipant alloc]
        initWithNativeParticipant:nativeParticipant];
  return participant;
}
@end
