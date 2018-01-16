/*
 * Intel License
 */

#include "talk/ics/sdk/conference/objc/RemoteMixedStreamObserverObjcImpl.h"

namespace ics {
namespace conference {
RemoteMixedStreamObserverObjcImpl::RemoteMixedStreamObserverObjcImpl(
    id<ICSRemoteMixedStreamObserver> observer) {
  observer_ = observer;
}

void RemoteMixedStreamObserverObjcImpl::OnVideoLayoutChanged(){
  [observer_ onVideoLayoutChanged];
}
}
}
