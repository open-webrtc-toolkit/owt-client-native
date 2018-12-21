// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "talk/oms/sdk/include/cpp/oms/base/commontypes.h"
#import "talk/oms/sdk/base/objc/OMSRemoteStream+Private.h"
#import "talk/oms/sdk/conference/objc/OMSConferenceInfo+Private.h"
#import "talk/oms/sdk/conference/objc/OMSConferenceParticipant+Private.h"
#import "talk/oms/sdk/include/objc/OMS/OMSRemoteMixedStream.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
@implementation OMSConferenceInfo
@dynamic conferenceId, participants, remoteStreams, myself;
- (instancetype)initWithNativeInfo:
    (std::shared_ptr<const oms::conference::ConferenceInfo>)info {
  self = [super init];
  _nativeInfo = info;
  return self;
}
- (NSString*)conferenceId {
  return [NSString stringForStdString:_nativeInfo->Id()];
}
- (NSArray<OMSConferenceParticipant*>*)participants {
  const auto& nativeParticipants = _nativeInfo->Participants();
  NSMutableArray<OMSConferenceParticipant*>* participants =
      [NSMutableArray arrayWithCapacity:nativeParticipants.size()];
  for (const auto& nativeParticipant : nativeParticipants) {
    OMSConferenceParticipant* participant = [[OMSConferenceParticipant alloc]
        initWithNativeParticipant:nativeParticipant];
    [participants addObject:participant];
  }
  return participants;
}
- (NSArray<OMSRemoteStream*>*)remoteStreams {
  const auto& nativeStreams = _nativeInfo->RemoteStreams();
  NSMutableArray<OMSRemoteStream*>* streams =
      [NSMutableArray arrayWithCapacity:nativeStreams.size()];
  for (const auto& nativeStream : nativeStreams) {
    if (nativeStream->Source().audio == oms::base::AudioSourceInfo::kMixed &&
        nativeStream->Source().video == oms::base::VideoSourceInfo::kMixed) {
      OMSRemoteMixedStream* stream =
          [[OMSRemoteMixedStream alloc] initWithNativeStream:nativeStream];
      [streams addObject:stream];
    } else {
      OMSRemoteStream* stream =
          [[OMSRemoteStream alloc] initWithNativeStream:nativeStream];
      [streams addObject:stream];
    }
  }
  return streams;
}
- (OMSConferenceParticipant*)myself {
  const auto& nativeParticipant = _nativeInfo->Self();
  OMSConferenceParticipant* participant = [[OMSConferenceParticipant alloc]
        initWithNativeParticipant:nativeParticipant];
  return participant;
}
@end
