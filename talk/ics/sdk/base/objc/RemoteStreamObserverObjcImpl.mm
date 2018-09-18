/*
 * Intel License
 */

#include "talk/ics/sdk/base/objc/RemoteStreamObserverObjcImpl.h"
#include "webrtc/rtc_base/checks.h"

namespace ics {
namespace base {
RemoteStreamObserverObjcImpl::RemoteStreamObserverObjcImpl(
    ICSRemoteStream* stream,
    id<ICSRemoteStreamDelegate> delegate)
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
