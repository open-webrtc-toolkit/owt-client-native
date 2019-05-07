// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_OBJC_REMOTESTREAMOBSERVEROBJCIMPL_H_
#define OWT_BASE_OBJC_REMOTESTREAMOBSERVEROBJCIMPL_H_
#import "talk/owt/sdk/include/cpp/owt/base/stream.h"
#import "talk/owt/sdk/include/objc/OWT/OWTRemoteStream.h"
namespace owt {
namespace base {
class RemoteStreamObserverObjcImpl : public StreamObserver {
 public:
  RemoteStreamObserverObjcImpl(OWTRemoteStream* stream,
                               id<OWTRemoteStreamDelegate> delegate);
 protected:
  virtual void OnEnded() override;
  virtual void OnUpdated() override;
  virtual void OnMute(owt::base::TrackKind track_kind) override;
  virtual void OnUnmute(owt::base::TrackKind track_kind) override;
 private:
  OWTRemoteStream* stream_;
  id<OWTRemoteStreamDelegate> delegate_;
};
}  // namespace base
}  // namespace owt
#endif  // OWT_BASE_OBJC_REMOTEMIXEDSTREAMOBSERVEROBJCIMPL_H_
