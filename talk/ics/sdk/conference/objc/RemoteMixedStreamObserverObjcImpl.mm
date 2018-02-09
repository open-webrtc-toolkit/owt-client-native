/*
 * Intel License
 */

#include "talk/ics/sdk/conference/objc/RemoteMixedStreamObserverObjcImpl.h"

namespace ics {
namespace conference {
RemoteMixedStreamObserverObjcImpl::RemoteMixedStreamObserverObjcImpl(
    ICSRemoteMixedStream* stream,
    id<ICSRemoteMixedStreamDelegate> delegate)
    : stream_(stream), delegate_(delegate) {}

void RemoteMixedStreamObserverObjcImpl::OnVideoLayoutChanged() {
  [delegate_ remoteMixedStreamDidChangeVideoLayout:stream_];
}
}
}
