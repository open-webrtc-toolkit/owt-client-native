/*
 * Intel License
 */
#include "talk/oms/sdk/conference/objc/RemoteMixedStreamObserverObjcImpl.h"
namespace oms {
namespace conference {
RemoteMixedStreamObserverObjcImpl::RemoteMixedStreamObserverObjcImpl(
    OMSRemoteMixedStream* stream,
    id<OMSRemoteMixedStreamDelegate> delegate)
    : stream_(stream), delegate_(delegate) {}
void RemoteMixedStreamObserverObjcImpl::OnVideoLayoutChanged() {
  if ([delegate_ respondsToSelector:@selector(streamDidChangeVideoLayout:)]) {
    [delegate_ streamDidChangeVideoLayout:stream_];
  }
}
}
}
