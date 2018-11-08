//
//  Copyright (c) 2018 Intel Corporation. All rights reserved.
//
#include "talk/oms/sdk/conference/objc/ConferencePublicationObserverObjcImpl.h"
#import "talk/oms/sdk/base/objc/OMSMediaFormat+Private.h"
namespace oms {
namespace conference {
void ConferencePublicationObserverObjcImpl::OnEnded() {
  if ([delegate_ respondsToSelector:@selector(publicationDidEnd:)]) {
    [delegate_ publicationDidEnd:publication_];
  }
}
void ConferencePublicationObserverObjcImpl::OnMute(
    oms::base::TrackKind track_kind) {
  if ([delegate_ respondsToSelector:@selector(publicationDidMute:trackKind:)]) {
    [delegate_ publicationDidMute:publication_
                        trackKind:[OMSTrackKindConverter
                                      objcTrackKindForCppTrackKind:track_kind]];
  }
}
void ConferencePublicationObserverObjcImpl::OnUnmute(
    oms::base::TrackKind track_kind) {
  if ([delegate_
          respondsToSelector:@selector(publicationDidUnmute:trackKind:)]) {
    [delegate_
        publicationDidUnmute:publication_
                   trackKind:[OMSTrackKindConverter
                                 objcTrackKindForCppTrackKind:track_kind]];
  }
}
}
}
