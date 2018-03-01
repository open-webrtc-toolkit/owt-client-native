/*
 * Intel License
 */

#include "talk/ics/sdk/base/objc/RemoteStreamObserverObjcImpl.h"

namespace ics {
namespace base {
RemoteStreamObserverObjcImpl::RemoteStreamObserverObjcImpl(
    ICSRemoteStream* stream,
    id<ICSRemoteStreamDelegate> delegate)
    : stream_(stream), delegate_(delegate) {}

void RemoteStreamObserverObjcImpl::OnEnded() {
  if ([delegate_ respondsToSelector:@selector(streamDidEnd:)]) {
    [delegate_ streamDidEnd:stream_];
  }
}
}
}
