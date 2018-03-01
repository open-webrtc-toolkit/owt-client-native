//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/include/objc/ICS/ICSRemoteStream.h"
#import <Foundation/Foundation.h>
#import "WebRTC/RTCMediaStream.h"
#import "talk/ics/sdk/base/objc/ICSRemoteStream+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
#import "talk/ics/sdk/base/objc/ICSMediaFormat+Private.h"

#include "talk/ics/sdk/base/objc/RemoteStreamObserverObjcImpl.h"

@implementation ICSRemoteStream {
  std::unique_ptr<ics::base::RemoteStreamObserverObjcImpl,
                  std::function<void(ics::base::RemoteStreamObserverObjcImpl*)>>
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

- (ICSPublicationSettings*)settings {
  return [[ICSPublicationSettings alloc]
      initWithNativePublicationSettings:std::static_pointer_cast<
                                            ics::base::RemoteStream>(
                                            [super nativeStream])
                                            ->Settings()];
}

-(ICSSubscriptionCapabilities*)capabilities{
  return [[ICSSubscriptionCapabilities alloc]
      initWithNativeSubscriptionCapabilities:std::static_pointer_cast<
                                            ics::base::RemoteStream>(
                                            [super nativeStream])
                                            ->Capabilities()];
}

- (std::shared_ptr<ics::base::RemoteStream>)nativeRemoteStream {
  std::shared_ptr<ics::base::Stream> stream = [super nativeStream];
  return std::static_pointer_cast<ics::base::RemoteStream>(stream);
}

- (void)setDelegate:(id<ICSRemoteStreamDelegate>)delegate {
  _observer = std::unique_ptr<
      ics::base::RemoteStreamObserverObjcImpl,
      std::function<void(ics::base::RemoteStreamObserverObjcImpl*)>>(
      new ics::base::RemoteStreamObserverObjcImpl(self, delegate),
      [&self](ics::base::RemoteStreamObserverObjcImpl* observer) {
        [self nativeRemoteStream] -> RemoveObserver(*_observer.get());
      });
  auto remoteStream = [self nativeRemoteStream];
  remoteStream->AddObserver(*_observer.get());
}

@end
