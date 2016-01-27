/*
 * Intel License
 */

#ifndef WOOGEEN_CONFERENCE_OBJC_REMOTEMIXEDSTREAMOBSERVEROBJCIMPL_H_
#define WOOGEEN_CONFERENCE_OBJC_REMOTEMIXEDSTREAMOBSERVEROBJCIMPL_H_

#import "talk/woogeen/sdk/include/cpp/woogeen/conference/remotemixedstream.h"
#import "talk/woogeen/sdk/conference/objc/public/RTCRemoteMixedStreamObserver.h"

namespace woogeen {
namespace conference {
class RemoteMixedStreamObserverObjcImpl : public RemoteMixedStreamObserver {
 public:
  RemoteMixedStreamObserverObjcImpl(id<RTCRemoteMixedStreamObserver> observer);

 protected:
  virtual void OnVideoLayoutChanged() override;

 private:
  id<RTCRemoteMixedStreamObserver> observer_;
};
}
}

#endif  // WOOGEEN_CONFERENCE_OBJC_REMOTEMIXEDSTREAMOBSERVEROBJCIMPL_H_
