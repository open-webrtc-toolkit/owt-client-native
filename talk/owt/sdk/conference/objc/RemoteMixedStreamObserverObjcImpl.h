// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_CONFERENCE_OBJC_REMOTEMIXEDSTREAMOBSERVEROBJCIMPL_H_
#define OWT_CONFERENCE_OBJC_REMOTEMIXEDSTREAMOBSERVEROBJCIMPL_H_
#import "talk/owt/sdk/include/cpp/owt/conference/remotemixedstream.h"
#import "talk/owt/sdk/include/objc/OWT/OWTRemoteMixedStream.h"
namespace owt {
namespace conference {
class RemoteMixedStreamObserverObjcImpl : public RemoteMixedStreamObserver {
 public:
  RemoteMixedStreamObserverObjcImpl(OWTRemoteMixedStream* stream,
                                    id<OWTRemoteMixedStreamDelegate> delegate);
 protected:
  virtual void OnVideoLayoutChanged() override;
  virtual void OnActiveInputChanged(const std::string& stream_id) override;
 private:
  OWTRemoteMixedStream* stream_;
  id<OWTRemoteMixedStreamDelegate> delegate_;
};
}  // namespace conference
}  // namespace owt
#endif  // OWT_CONFERENCE_OBJC_REMOTEMIXEDSTREAMOBSERVEROBJCIMPL_H_
