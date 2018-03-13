#ifndef ICS_CONFERENCE_OBJC_CONFERENCESUBSCRIPTIONOBJCIMPL_H_
#define ICS_CONFERENCE_OBJC_CONFERENCESUBSCRIPTIONOBJCIMPL_H_

#include "talk/ics/sdk/include/cpp/ics/base/subscription.h"

#import "talk/ics/sdk/include/objc/ICS/ICSConferenceSubscription.h"

namespace ics {
namespace conference {
class ConferenceSubscriptionObserverObjcImpl
    : public ics::base::SubscriptionObserver {
 public:
  ConferenceSubscriptionObserverObjcImpl(
      ICSConferenceSubscription* subscription,
      id<ICSConferenceSubscriptionDelegate> delegate)
      : subscription_(subscription), delegate_(delegate) {}

 protected:
  /// Triggered when publication is ended.
  virtual void OnEnded() override;
  /// Triggered when audio and/or video is muted.
  virtual void OnMute(ics::base::TrackKind track_kind) override;
  /// Triggered when audio and/or video is unmuted.
  virtual void OnUnmute(ics::base::TrackKind track_kind) override;

 private:
  ICSConferenceSubscription* subscription_;
  id<ICSConferenceSubscriptionDelegate> delegate_;
};
}  // namespace conference
}  // namespace ics

#endif
