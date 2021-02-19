// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "talk/owt/sdk/include/objc/OWT/OWTRemoteStream.h"
#import <Foundation/Foundation.h>
#import "RTCMediaStream.h"
#import "talk/owt/sdk/base/objc/OWTRemoteStream+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
#import "talk/owt/sdk/base/objc/OWTMediaFormat+Private.h"
#include "talk/owt/sdk/base/objc/RemoteStreamObserverObjcImpl.h"
@implementation OWTRemoteStream {
  std::unique_ptr<owt::base::RemoteStreamObserverObjcImpl,
                  std::function<void(owt::base::RemoteStreamObserverObjcImpl*)>>
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

- (OWTPublicationSettings*)settings {
  auto nativeSettings =
      std::static_pointer_cast<owt::base::RemoteStream>([super nativeStream])
          ->Settings();
  return [[OWTPublicationSettings alloc]
      initWithNativePublicationSettings:nativeSettings];
}

-(OWTSubscriptionCapabilities*)capabilities{
  return [[OWTSubscriptionCapabilities alloc]
      initWithNativeSubscriptionCapabilities:std::static_pointer_cast<
                                            owt::base::RemoteStream>(
                                            [super nativeStream])
                                            ->Capabilities()];
}
- (std::shared_ptr<owt::base::RemoteStream>)nativeRemoteStream {
  std::shared_ptr<owt::base::Stream> stream = [super nativeStream];
  return std::static_pointer_cast<owt::base::RemoteStream>(stream);
}
- (void)setDelegate:(id<OWTRemoteStreamDelegate>)delegate {
  _observer = std::unique_ptr<owt::base::RemoteStreamObserverObjcImpl>(
      new owt::base::RemoteStreamObserverObjcImpl(self, delegate));
  auto remoteStream = [self nativeRemoteStream];
  remoteStream->AddObserver(*_observer.get());
}

- (void)dealloc {
  if (_observer) {
    [self nativeRemoteStream]->RemoveObserver(*_observer.get());
  }
}

@end
