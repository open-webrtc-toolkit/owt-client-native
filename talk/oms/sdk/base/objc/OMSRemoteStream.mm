// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "talk/oms/sdk/include/objc/OMS/OMSRemoteStream.h"
#import <Foundation/Foundation.h>
#import "WebRTC/RTCMediaStream.h"
#import "talk/oms/sdk/base/objc/OMSRemoteStream+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
#import "talk/oms/sdk/base/objc/OMSMediaFormat+Private.h"
#include "talk/oms/sdk/base/objc/RemoteStreamObserverObjcImpl.h"
@implementation OMSRemoteStream {
  std::unique_ptr<oms::base::RemoteStreamObserverObjcImpl,
                  std::function<void(oms::base::RemoteStreamObserverObjcImpl*)>>
      _observer;
}
@dynamic delegate;
- (NSString*)streamId {
  auto remoteStream = [self nativeRemoteStream];
  return [NSString stringForStdString:remoteStream->Id()];
}
- (NSString*)origin {
  auto remoteStream = [self nativeRemoteStream];
  return [NSString stringForStdString:remoteStream->Origin()];
}
- (OMSPublicationSettings*)settings {
  return [[OMSPublicationSettings alloc]
      initWithNativePublicationSettings:std::static_pointer_cast<
                                            oms::base::RemoteStream>(
                                            [super nativeStream])
                                            ->Settings()];
}
-(OMSSubscriptionCapabilities*)capabilities{
  return [[OMSSubscriptionCapabilities alloc]
      initWithNativeSubscriptionCapabilities:std::static_pointer_cast<
                                            oms::base::RemoteStream>(
                                            [super nativeStream])
                                            ->Capabilities()];
}
- (std::shared_ptr<oms::base::RemoteStream>)nativeRemoteStream {
  std::shared_ptr<oms::base::Stream> stream = [super nativeStream];
  return std::static_pointer_cast<oms::base::RemoteStream>(stream);
}
- (void)setDelegate:(id<OMSRemoteStreamDelegate>)delegate {
  _observer = std::unique_ptr<
      oms::base::RemoteStreamObserverObjcImpl,
      std::function<void(oms::base::RemoteStreamObserverObjcImpl*)>>(
      new oms::base::RemoteStreamObserverObjcImpl(self, delegate),
      [&self](oms::base::RemoteStreamObserverObjcImpl* observer) {
        [self nativeRemoteStream] -> RemoveObserver(*_observer.get());
      });
  auto remoteStream = [self nativeRemoteStream];
  remoteStream->AddObserver(*_observer.get());
}
@end
