// Copyright (C) <2019> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_CONFERENCE_OBJC_PARTICIPANTOBSERVEROBJCIMPL_H_
#define OWT_CONFERENCE_OBJC_PARTICIPANTOBSERVEROBJCIMPL_H_

#import "talk/owt/sdk/include/cpp/owt/conference/conferenceclient.h"
#import "talk/owt/sdk/include/objc/OWT/OWTConferenceParticipant.h"

namespace owt {
namespace conference {
class ParticipantObserverObjcImpl : public ParticipantObserver {
 public:
  ParticipantObserverObjcImpl(OWTConferenceParticipant* participant,
                              id<OWTConferenceParticipantDelegate> delegate)
      : participant_(participant), delegate_(delegate) {}

 protected:
  virtual void OnLeft() override;

 private:
  OWTConferenceParticipant* participant_;
  id<OWTConferenceParticipantDelegate> delegate_;
};
}  // namespace conference
}  // namespace owt

#endif