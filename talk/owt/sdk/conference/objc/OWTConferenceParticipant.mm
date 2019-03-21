// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "talk/owt/sdk/conference/objc/OWTConferenceParticipant+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
@implementation OWTConferenceParticipant
- (instancetype)initWithNativeParticipant:
    (std::shared_ptr<const owt::conference::Participant>)participant {
  self = [super init];
  _nativeParticipant = participant;
  return self;
}
- (NSString*)participantId {
  return [NSString stringForStdString:_nativeParticipant->Id()];
}
- (NSString*)role {
  return [NSString stringForStdString:_nativeParticipant->Role()];
}
- (NSString*)userId {
  return [NSString stringForStdString:_nativeParticipant->UserId()];
}
@end
