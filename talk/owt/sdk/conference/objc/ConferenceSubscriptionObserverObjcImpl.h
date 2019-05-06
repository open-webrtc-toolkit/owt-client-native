// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_CONFERENCE_OBJC_CONFERENCESUBSCRIPTIONOBJCIMPL_H_
#define OWT_CONFERENCE_OBJC_CONFERENCESUBSCRIPTIONOBJCIMPL_H_
#include "talk/owt/sdk/include/cpp/owt/base/exception.h"
#include "talk/owt/sdk/include/cpp/owt/base/subscription.h"
#import "talk/owt/sdk/include/objc/OWT/OWTConferenceSubscription.h"
namespace owt {
namespace conference {
class ConferenceSubscriptionObserverObjcImpl
    : public owt::base::SubscriptionObserver {
 public:
  ConferenceSubscriptionObserverObjcImpl(
      OWTConferenceSubscription* subscription,
      id<OWTConferenceSubscriptionDelegate> delegate)
      : subscription_(subscription), delegate_(delegate) {}
  virtual ~ConferenceSubscriptionObserverObjcImpl(){}
 protected:
  /// Triggered when publication is ended.
  virtual void OnEnded() override;
  /// Triggered when audio and/or video is muted.
  virtual void OnMute(owt::base::TrackKind track_kind) override;
  /// Triggered when audio and/or video is unmuted.
  virtual void OnUnmute(owt::base::TrackKind track_kind) override;
  /// Triggered when error occurs on subscription.
  virtual void OnError(std::unique_ptr<owt::base::Exception> error_info) override;
 private:
  __weak OWTConferenceSubscription* subscription_;
  __weak id<OWTConferenceSubscriptionDelegate> delegate_;
};
}  // namespace conference
}  // namespace owt
#endif
