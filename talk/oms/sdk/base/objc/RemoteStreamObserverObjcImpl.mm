// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "talk/oms/sdk/base/objc/RemoteStreamObserverObjcImpl.h"
#include "webrtc/rtc_base/checks.h"
namespace oms {
namespace base {
RemoteStreamObserverObjcImpl::RemoteStreamObserverObjcImpl(
    OMSRemoteStream* stream,
    id<OMSRemoteStreamDelegate> delegate)
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
