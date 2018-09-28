//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/conference/objc/ICSConferenceParticipant+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"

@implementation ICSConferenceParticipant

- (instancetype)initWithNativeParticipant:
    (std::shared_ptr<const ics::conference::Participant>)participant {
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
