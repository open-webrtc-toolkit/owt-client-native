/*
 * Intel License
 */

#ifndef WOOGEEN_CONFERENCE_OBJC_REMOTEMIXEDSTREAMOBSERVEROBJCIMPL_H_
#define WOOGEEN_CONFERENCE_OBJC_REMOTEMIXEDSTREAMOBSERVEROBJCIMPL_H_

#import "talk/ics/sdk/include/cpp/ics/conference/remotemixedstream.h"
#import "talk/ics/sdk/include/objc/Woogeen/RTCRemoteMixedStreamObserver.h"

namespace ics {
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
