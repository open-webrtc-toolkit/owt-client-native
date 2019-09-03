// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "talk/owt/sdk/include/objc/OWT/OWTRemoteStream.h"
#import <Foundation/Foundation.h>
#import "WebRTC/RTCMediaStream.h"
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
- (NSArray<OWTPublicationSettings*>*)settings {
  auto nativeSettings = std::static_pointer_cast<
                                            owt::base::RemoteStream>(
                                            [super nativeStream])
                                            ->Settings();
  NSMutableArray<OWTPublicationSettings*>* settingsArray =
      [NSMutableArray arrayWithCapacity:nativeSettings.size()];
  for (auto& s : nativeSettings) {
    OWTPublicationSettings* p =
      [[OWTPublicationSettings alloc] initWithNativePublicationSettings:s];
    [settingsArray addObject:p];
  }
  return settingsArray;
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
  _observer = std::unique_ptr<
      owt::base::RemoteStreamObserverObjcImpl,
      std::function<void(owt::base::RemoteStreamObserverObjcImpl*)>>(
      new owt::base::RemoteStreamObserverObjcImpl(self, delegate),
      [&self](owt::base::RemoteStreamObserverObjcImpl* observer) {
        [self nativeRemoteStream] -> RemoveObserver(*_observer.get());
      });
  auto remoteStream = [self nativeRemoteStream];
  remoteStream->AddObserver(*_observer.get());
}
@end
