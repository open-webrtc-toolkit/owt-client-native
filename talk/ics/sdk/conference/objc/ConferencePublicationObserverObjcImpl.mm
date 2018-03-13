//
//  Copyright (c) 2018 Intel Corporation. All rights reserved.
//

#include "talk/ics/sdk/conference/objc/ConferencePublicationObserverObjcImpl.h"

#import "talk/ics/sdk/base/objc/ICSMediaFormat+Private.h"

namespace ics {
namespace conference {
void ConferencePublicationObserverObjcImpl::OnEnded() {
  if ([delegate_ respondsToSelector:@selector(publicationDidEnd:)]) {
    [delegate_ publicationDidEnd:publication_];
  }
}

void ConferencePublicationObserverObjcImpl::OnMute(
    ics::base::TrackKind track_kind) {
  if ([delegate_ respondsToSelector:@selector(publicationDidMute:trackKind:)]) {
    [delegate_ publicationDidMute:publication_
                        trackKind:[ICSTrackKindConverter
                                      objcTrackKindForCppTrackKind:track_kind]];
  }
}

void ConferencePublicationObserverObjcImpl::OnUnmute(
    ics::base::TrackKind track_kind) {
  if ([delegate_
          respondsToSelector:@selector(publicationDidUnmute:trackKind:)]) {
    [delegate_
        publicationDidUnmute:publication_
                   trackKind:[ICSTrackKindConverter
                                 objcTrackKindForCppTrackKind:track_kind]];
  }
}
}
}
