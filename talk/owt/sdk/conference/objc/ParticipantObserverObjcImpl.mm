// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#import "talk/owt/sdk/conference/objc/ParticipantObserverObjcImpl.h"

namespace owt {
namespace conference {
void ParticipantObserverObjcImpl::OnLeft() {
  if ([delegate_ respondsToSelector:@selector(participantDidLeave:)]) {
    [delegate_ participantDidLeave:participant_];
  }
}
}
}