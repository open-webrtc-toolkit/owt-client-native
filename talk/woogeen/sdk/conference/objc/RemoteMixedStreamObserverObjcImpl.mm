/*
 * Intel License
 */

#include "talk/woogeen/sdk/conference/objc/RemoteMixedStreamObserverObjcImpl.h"

namespace woogeen {
namespace conference {
RemoteMixedStreamObserverObjcImpl::RemoteMixedStreamObserverObjcImpl(
    id<RTCRemoteMixedStreamObserver> observer) {
  observer_ = observer;
}

void RemoteMixedStreamObserverObjcImpl::OnVideoLayoutChanged(){
  [observer_ onVideoLayoutChanged];
}
}
}
