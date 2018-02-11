//
//  Copyright (c) 2018 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/conference/objc/ICSConferenceInfo+Private.h"
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
  return [NSString stringForStdString:_nativeInfo->id];
}

- (NSArray<ICSConferenceParticipant*>*)participants {
  // TODO: Implement it after conference info is implemented in C++.
  return nil;
}

- (NSArray<ICSRemoteStream*>*)remoteStreams {
  // TODO: Implement it after conference info is implemented in C++.
  return nil;
}

- (ICSConferenceParticipant*)myself {
  // TODO: Implement it after conference info is implemented in C++.
  return nil;
}

@end
