//
//  Copyright (c) 2018 Intel Corporation. All rights reserved.
//

#include "talk/ics/sdk/conference/objc/ConferenceSubscriptionObserverObjcImpl.h"

#import "talk/ics/sdk/base/objc/ICSMediaFormat+Private.h"

namespace ics {
namespace conference {
void ConferenceSubscriptionObserverObjcImpl::OnEnded() {
  if ([delegate_ respondsToSelector:@selector(subscriptionDidEnd:)]) {
    [delegate_ subscriptionDidEnd:subscription_];
  }
}

void ConferenceSubscriptionObserverObjcImpl::OnMute(
    ics::base::TrackKind track_kind) {
  if ([delegate_
          respondsToSelector:@selector(subscriptionDidMute:trackKind:)]) {
    [delegate_
        subscriptionDidMute:subscription_
                  trackKind:[ICSTrackKindConverter
                                objcTrackKindForCppTrackKind:track_kind]];
  }
}

void ConferenceSubscriptionObserverObjcImpl::OnUnmute(
    ics::base::TrackKind track_kind) {
  if ([delegate_
          respondsToSelector:@selector(subscriptionDidUnmute:trackKind:)]) {
    [delegate_
        subscriptionDidUnmute:subscription_
                    trackKind:[ICSTrackKindConverter
                                  objcTrackKindForCppTrackKind:track_kind]];
  }
}
}
}
