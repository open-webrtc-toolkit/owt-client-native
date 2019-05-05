// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
#include "talk/owt/sdk/conference/objc/ConferenceSubscriptionObserverObjcImpl.h"
#import "talk/owt/sdk/base/objc/OWTMediaFormat+Private.h"
#import "talk/owt/sdk/include/objc/OWT/OWTConferenceErrors.h"
#import "talk/owt/sdk/include/objc/OWT/OWTErrors.h"
namespace owt {
namespace conference {
void ConferenceSubscriptionObserverObjcImpl::OnEnded() {
  if ([delegate_ respondsToSelector:@selector(subscriptionDidEnd:)]) {
    [delegate_ subscriptionDidEnd:subscription_];
  }
}
void ConferenceSubscriptionObserverObjcImpl::OnMute(
    owt::base::TrackKind track_kind) {
  if ([delegate_
          respondsToSelector:@selector(subscriptionDidMute:trackKind:)]) {
    [delegate_
        subscriptionDidMute:subscription_
                  trackKind:[OWTTrackKindConverter
                                objcTrackKindForCppTrackKind:track_kind]];
  }
}
void ConferenceSubscriptionObserverObjcImpl::OnUnmute(
    owt::base::TrackKind track_kind) {
  if ([delegate_
          respondsToSelector:@selector(subscriptionDidUnmute:trackKind:)]) {
    [delegate_
        subscriptionDidUnmute:subscription_
                    trackKind:[OWTTrackKindConverter
                                  objcTrackKindForCppTrackKind:track_kind]];
  }
}
void ConferenceSubscriptionObserverObjcImpl::OnError(
    std::unique_ptr<owt::base::Exception> error_info) {
  if ([delegate_
          respondsToSelector:@selector(subscriptionDidError:errorInfo:)]) {
    NSError* err = [[NSError alloc]
      initWithDomain:OWTErrorDomain
                code:OWTConferenceErrorUnknown
            userInfo:[[NSDictionary alloc]
                         initWithObjectsAndKeys:
                             [NSString stringForStdString:error_info->Message()],
                             NSLocalizedDescriptionKey, nil]];
    [delegate_
        subscriptionDidError:subscription_
                   errorInfo:err];
  }
}
}
}
