// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <Foundation/Foundation.h>
#import <functional>
#import "talk/owt/sdk/base/objc/OWTStream+Private.h"
#import "talk/owt/sdk/conference/objc/RemoteMixedStreamObserverObjcImpl.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"

@implementation OWTRemoteMixedStream {
  std::unique_ptr<owt::conference::RemoteMixedStreamObserverObjcImpl,
                  std::function<void(owt::conference::RemoteMixedStreamObserverObjcImpl*)>>
      _observer;
}
@dynamic delegateMix;
- (NSString*)viewport {
  std::shared_ptr<owt::conference::RemoteMixedStream> stream_ptr =
      std::static_pointer_cast<owt::conference::RemoteMixedStream>(
          [self nativeStream]);
  return [NSString stringForStdString:stream_ptr->Viewport()];
}
- (std::shared_ptr<owt::conference::RemoteMixedStream>)nativeRemoteStream {
  std::shared_ptr<owt::base::Stream> stream = [super nativeStream];
  return std::static_pointer_cast<owt::conference::RemoteMixedStream>(stream);
}
- (void)setDelegateMix:(id<OWTRemoteMixedStreamDelegate>)delegateMix {
  _observer =
      std::unique_ptr<owt::conference::RemoteMixedStreamObserverObjcImpl>(
          new owt::conference::RemoteMixedStreamObserverObjcImpl(self,
                                                                 delegateMix));
  auto remoteStream = [self nativeRemoteStream];
  remoteStream->AddObserver(*_observer.get());
}

- (void)dealloc {
  if (_observer) {
    [self nativeRemoteStream]->RemoveObserver(*_observer.get());
  }
}

@end
