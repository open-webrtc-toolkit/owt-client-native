// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
#include "talk/owt/sdk/conference/objc/ConferencePublicationObserverObjcImpl.h"
#import "talk/owt/sdk/base/objc/OWTMediaFormat+Private.h"
#import "talk/owt/sdk/include/objc/OWT/OWTConferenceErrors.h"
#import "talk/owt/sdk/include/objc/OWT/OWTErrors.h"
namespace owt {
namespace conference {
void ConferencePublicationObserverObjcImpl::OnEnded() {
  if ([delegate_ respondsToSelector:@selector(publicationDidEnd:)]) {
    [delegate_ publicationDidEnd:publication_];
  }
}
void ConferencePublicationObserverObjcImpl::OnMute(
    owt::base::TrackKind track_kind) {
  if ([delegate_ respondsToSelector:@selector(publicationDidMute:trackKind:)]) {
    [delegate_ publicationDidMute:publication_
                        trackKind:[OWTTrackKindConverter
                                      objcTrackKindForCppTrackKind:track_kind]];
  }
}
void ConferencePublicationObserverObjcImpl::OnUnmute(
    owt::base::TrackKind track_kind) {
  if ([delegate_
          respondsToSelector:@selector(publicationDidUnmute:trackKind:)]) {
    [delegate_
        publicationDidUnmute:publication_
                   trackKind:[OWTTrackKindConverter
                                 objcTrackKindForCppTrackKind:track_kind]];
  }
}
void ConferencePublicationObserverObjcImpl::OnError(
    std::unique_ptr<owt::base::Exception> error_info) {
  if ([delegate_
          respondsToSelector:@selector(publicationDidError:errorInfo:)]) {
    NSError* err = [[NSError alloc]
      initWithDomain:OWTErrorDomain
                code:OWTConferenceErrorUnknown
            userInfo:[[NSDictionary alloc]
                         initWithObjectsAndKeys:
                             [NSString stringForStdString:error_info->Message()],
                             NSLocalizedDescriptionKey, nil]];
    [delegate_
        publicationDidError:publication_
                    errorInfo:err];
  }
}
}
}
