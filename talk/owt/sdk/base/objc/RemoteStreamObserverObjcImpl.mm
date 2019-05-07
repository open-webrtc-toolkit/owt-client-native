// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "talk/owt/sdk/base/objc/OWTMediaFormat+Private.h"
#include "talk/owt/sdk/base/objc/RemoteStreamObserverObjcImpl.h"
#include "webrtc/rtc_base/checks.h"
namespace owt {
namespace base {
RemoteStreamObserverObjcImpl::RemoteStreamObserverObjcImpl(
    OWTRemoteStream* stream,
    id<OWTRemoteStreamDelegate> delegate)
    : stream_(stream), delegate_(delegate) {
  RTC_CHECK(stream);
  RTC_CHECK(delegate);
}
void RemoteStreamObserverObjcImpl::OnEnded() {
  if ([delegate_ respondsToSelector:@selector(streamDidEnd:)]) {
    [delegate_ streamDidEnd:stream_];
  }
}
void RemoteStreamObserverObjcImpl::OnUpdated() {
  if ([delegate_ respondsToSelector:@selector(streamDidUpdate:)]) {
    [delegate_ streamDidUpdate:stream_];
  }
}
void RemoteStreamObserverObjcImpl::OnMute(owt::base::TrackKind track_kind) {
  if ([delegate_ respondsToSelector:@selector(streamDidMute:trackKind:)]) {
    [delegate_ streamDidMute:stream_ 
                  trackKind:[OWTTrackKindConverter
                                objcTrackKindForCppTrackKind:track_kind]];
  }
}
void RemoteStreamObserverObjcImpl::OnUnmute(owt::base::TrackKind track_kind) {
  if ([delegate_ respondsToSelector:@selector(streamDidUnmute:trackKind:)]) {
    [delegate_ streamDidUnmute:stream_ 
                  trackKind:[OWTTrackKindConverter
                                objcTrackKindForCppTrackKind:track_kind]];
  }
}
}
}
