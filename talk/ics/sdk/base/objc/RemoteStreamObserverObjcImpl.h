/*
 * Intel License
 */

#ifndef ICS_BASE_OBJC_REMOTESTREAMOBSERVEROBJCIMPL_H_
#define ICS_BASE_OBJC_REMOTESTREAMOBSERVEROBJCIMPL_H_

#import "talk/ics/sdk/include/cpp/ics/base/stream.h"
#import "talk/ics/sdk/include/objc/ICS/ICSRemoteStream.h"

namespace ics {
namespace base {
class RemoteStreamObserverObjcImpl : public StreamObserver {
 public:
  RemoteStreamObserverObjcImpl(ICSRemoteStream* stream,
                               id<ICSRemoteStreamDelegate> delegate);

 protected:
  virtual void OnEnded() override;

 private:
  ICSRemoteStream* stream_;
  id<ICSRemoteStreamDelegate> delegate_;
};
}  // namespace base
}  // namespace ics

#endif  // ICS_BASE_OBJC_REMOTEMIXEDSTREAMOBSERVEROBJCIMPL_H_
