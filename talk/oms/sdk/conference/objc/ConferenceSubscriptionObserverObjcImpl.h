#ifndef OMS_CONFERENCE_OBJC_CONFERENCESUBSCRIPTIONOBJCIMPL_H_
#define OMS_CONFERENCE_OBJC_CONFERENCESUBSCRIPTIONOBJCIMPL_H_

#include "talk/oms/sdk/include/cpp/oms/base/subscription.h"

#import "talk/oms/sdk/include/objc/OMS/OMSConferenceSubscription.h"

namespace oms {
namespace conference {
class ConferenceSubscriptionObserverObjcImpl
    : public oms::base::SubscriptionObserver {
 public:
  ConferenceSubscriptionObserverObjcImpl(
      OMSConferenceSubscription* subscription,
      id<OMSConferenceSubscriptionDelegate> delegate)
      : subscription_(subscription), delegate_(delegate) {}

 protected:
  /// Triggered when publication is ended.
  virtual void OnEnded() override;
  /// Triggered when audio and/or video is muted.
  virtual void OnMute(oms::base::TrackKind track_kind) override;
  /// Triggered when audio and/or video is unmuted.
  virtual void OnUnmute(oms::base::TrackKind track_kind) override;

 private:
  OMSConferenceSubscription* subscription_;
  id<OMSConferenceSubscriptionDelegate> delegate_;
};
}  // namespace conference
}  // namespace oms

#endif
