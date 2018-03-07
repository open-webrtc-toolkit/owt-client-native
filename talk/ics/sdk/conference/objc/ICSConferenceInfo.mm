//
//  Copyright (c) 2018 Intel Corporation. All rights reserved.
//

#include "talk/ics/sdk/include/cpp/ics/base/commontypes.h"

#import "talk/ics/sdk/conference/objc/ICSConferenceInfo+Private.h"
#import "talk/ics/sdk/conference/objc/ICSConferenceParticipant+Private.h"
#import "talk/ics/sdk/conference/objc/ICSRemoteMixedStream+Internal.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"

@implementation ICSConferenceInfo

@dynamic conferenceId, participants, remoteStreams, myself;

- (instancetype)initWithNativeInfo:
    (std::shared_ptr<const ics::conference::ConferenceInfo>)info {
  self = [super init];
  _nativeInfo = info;
  return self;
}

- (NSString*)conferenceId {
  // TODO: Implement it after conference info is implemented in C++.
  return nil;
}

- (NSArray<ICSConferenceParticipant*>*)participants {
  const auto& nativeParticipants = _nativeInfo->Participants();
  NSMutableArray<ICSConferenceParticipant*>* participants =
      [NSMutableArray arrayWithCapacity:nativeParticipants.size()];
  for (const auto& nativeParticipant : nativeParticipants) {
    ICSConferenceParticipant* participant = [[ICSConferenceParticipant alloc]
        initWithNativeParticipant:nativeParticipant];
    [participants addObject:participant];
  }
  return participants;
}

- (NSArray<ICSRemoteStream*>*)remoteStreams {
  const auto& nativeStreams = _nativeInfo->RemoteStreams();
  NSMutableArray<ICSRemoteStream*>* streams =
      [NSMutableArray arrayWithCapacity:nativeStreams.size()];
  for (const auto& nativeStream : nativeStreams) {
    if (nativeStream->Source().audio == ics::base::AudioSourceInfo::kMixed &&
        nativeStream->Source().video == ics::base::VideoSourceInfo::kMixed) {
      ICSRemoteMixedStream* stream =
          [[ICSRemoteMixedStream alloc] initWithNativeStream:nativeStream];
      [streams addObject:stream];
    } else {
      ICSRemoteStream* stream =
          [[ICSRemoteStream alloc] initWithNativeStream:nativeStream];
      [streams addObject:stream];
    }
  }
  return streams;
}

- (ICSConferenceParticipant*)myself {
  // TODO: Implement it after conference info is implemented in C++.
  return nil;
}

@end
