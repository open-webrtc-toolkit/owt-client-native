// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OMS_BASE_OBJC_REMOTESTREAMOBSERVEROBJCIMPL_H_
#define OMS_BASE_OBJC_REMOTESTREAMOBSERVEROBJCIMPL_H_
#import "talk/oms/sdk/include/cpp/oms/base/stream.h"
#import "talk/oms/sdk/include/objc/OMS/OMSRemoteStream.h"
namespace oms {
namespace base {
class RemoteStreamObserverObjcImpl : public StreamObserver {
 public:
  RemoteStreamObserverObjcImpl(OMSRemoteStream* stream,
                               id<OMSRemoteStreamDelegate> delegate);
 protected:
  virtual void OnEnded() override;
  virtual void OnUpdated() override;
 private:
  OMSRemoteStream* stream_;
  id<OMSRemoteStreamDelegate> delegate_;
};
}  // namespace base
}  // namespace oms
#endif  // OMS_BASE_OBJC_REMOTEMIXEDSTREAMOBSERVEROBJCIMPL_H_
