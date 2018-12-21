// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OMS_CONFERENCE_OBJC_REMOTEMIXEDSTREAMOBSERVEROBJCIMPL_H_
#define OMS_CONFERENCE_OBJC_REMOTEMIXEDSTREAMOBSERVEROBJCIMPL_H_
#import "talk/oms/sdk/include/cpp/oms/conference/remotemixedstream.h"
#import "talk/oms/sdk/include/objc/OMS/OMSRemoteMixedStream.h"
namespace oms {
namespace conference {
class RemoteMixedStreamObserverObjcImpl : public RemoteMixedStreamObserver {
 public:
  RemoteMixedStreamObserverObjcImpl(OMSRemoteMixedStream* stream,
                                    id<OMSRemoteMixedStreamDelegate> delegate);
 protected:
  virtual void OnVideoLayoutChanged() override;
 private:
  OMSRemoteMixedStream* stream_;
  id<OMSRemoteMixedStreamDelegate> delegate_;
};
}  // namespace conference
}  // namespace oms
#endif  // OMS_CONFERENCE_OBJC_REMOTEMIXEDSTREAMOBSERVEROBJCIMPL_H_
