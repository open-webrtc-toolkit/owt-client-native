//
//  Copyright (c) 2018 Intel Corporation. All rights reserved.
//

#include "talk/oms/sdk/conference/objc/ConferenceSubscriptionObserverObjcImpl.h"

#import "talk/oms/sdk/base/objc/OMSMediaFormat+Private.h"

namespace oms {
namespace conference {
void ConferenceSubscriptionObserverObjcImpl::OnEnded() {
  if ([delegate_ respondsToSelector:@selector(subscriptionDidEnd:)]) {
    [delegate_ subscriptionDidEnd:subscription_];
  }
}

void ConferenceSubscriptionObserverObjcImpl::OnMute(
    oms::base::TrackKind track_kind) {
  if ([delegate_
          respondsToSelector:@selector(subscriptionDidMute:trackKind:)]) {
    [delegate_
        subscriptionDidMute:subscription_
                  trackKind:[OMSTrackKindConverter
                                objcTrackKindForCppTrackKind:track_kind]];
  }
}

void ConferenceSubscriptionObserverObjcImpl::OnUnmute(
    oms::base::TrackKind track_kind) {
  if ([delegate_
          respondsToSelector:@selector(subscriptionDidUnmute:trackKind:)]) {
    [delegate_
        subscriptionDidUnmute:subscription_
                    trackKind:[OMSTrackKindConverter
                                  objcTrackKindForCppTrackKind:track_kind]];
  }
}
}
}
