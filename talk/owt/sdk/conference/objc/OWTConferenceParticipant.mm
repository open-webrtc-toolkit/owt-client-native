// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "talk/owt/sdk/conference/objc/OWTConferenceParticipant+Private.h"
#import "talk/owt/sdk/conference/objc/ParticipantObserverObjcImpl.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
@implementation OWTConferenceParticipant {
  std::unique_ptr<
      owt::conference::ParticipantObserverObjcImpl,
      std::function<void(owt::conference::ParticipantObserverObjcImpl*)>>
      _observer;
}
- (instancetype)initWithNativeParticipant:
    (std::shared_ptr<owt::conference::Participant>)participant {
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

- (void)setDelegate:(id<OWTConferenceParticipantDelegate>) delegate {
  _observer = std::unique_ptr<
      owt::conference::ParticipantObserverObjcImpl,
      std::function<void(owt::conference::ParticipantObserverObjcImpl*)>>(
      new owt::conference::ParticipantObserverObjcImpl(self, delegate),
      [&self](owt::conference::ParticipantObserverObjcImpl* observer) {
        self->_nativeParticipant->RemoveObserver(*observer);
      });
  _nativeParticipant->AddObserver(*_observer.get());
  _delegate = delegate;
}
@end
