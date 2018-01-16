/*
 * Intel License
 */

#ifndef ICS_CONFERENCE_OBJC_REMOTEMIXEDSTREAMOBSERVEROBJCIMPL_H_
#define ICS_CONFERENCE_OBJC_REMOTEMIXEDSTREAMOBSERVEROBJCIMPL_H_

#import "talk/ics/sdk/include/cpp/ics/conference/remotemixedstream.h"
#import "talk/ics/sdk/include/objc/ICS/ICSRemoteMixedStreamObserver.h"

namespace ics {
namespace conference {
class RemoteMixedStreamObserverObjcImpl : public RemoteMixedStreamObserver {
 public:
  RemoteMixedStreamObserverObjcImpl(id<ICSRemoteMixedStreamObserver> observer);

 protected:
  virtual void OnVideoLayoutChanged() override;

 private:
  id<ICSRemoteMixedStreamObserver> observer_;
};
}
}

#endif  // ICS_CONFERENCE_OBJC_REMOTEMIXEDSTREAMOBSERVEROBJCIMPL_H_
