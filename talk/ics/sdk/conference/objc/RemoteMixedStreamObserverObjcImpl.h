/*
 * Intel License
 */

#ifndef ICS_CONFERENCE_OBJC_REMOTEMIXEDSTREAMOBSERVEROBJCIMPL_H_
#define ICS_CONFERENCE_OBJC_REMOTEMIXEDSTREAMOBSERVEROBJCIMPL_H_

#import "talk/ics/sdk/include/cpp/ics/conference/remotemixedstream.h"
#import "talk/ics/sdk/include/objc/ICS/ICSRemoteMixedStream.h"

namespace ics {
namespace conference {
class RemoteMixedStreamObserverObjcImpl : public RemoteMixedStreamObserver {
 public:
  RemoteMixedStreamObserverObjcImpl(ICSRemoteMixedStream* stream,
                                    id<ICSRemoteMixedStreamDelegate> delegate);

 protected:
  virtual void OnVideoLayoutChanged() override;

 private:
  ICSRemoteMixedStream* stream_;
  id<ICSRemoteMixedStreamDelegate> delegate_;
};
}  // namespace conference
}  // namespace ics

#endif  // ICS_CONFERENCE_OBJC_REMOTEMIXEDSTREAMOBSERVEROBJCIMPL_H_
