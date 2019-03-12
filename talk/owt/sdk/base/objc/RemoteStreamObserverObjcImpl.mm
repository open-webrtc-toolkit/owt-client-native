// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
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
}
}
