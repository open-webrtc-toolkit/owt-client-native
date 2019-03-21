// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <Foundation/Foundation.h>
#import "talk/owt/sdk/base/objc/OWTStream+Private.h"
#import "talk/owt/sdk/conference/objc/RemoteMixedStreamObserverObjcImpl.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
@implementation OWTRemoteMixedStream {
  std::vector<owt::conference::RemoteMixedStreamObserverObjcImpl*> _observers;
}
@synthesize delegate;
- (NSString*)viewport {
  std::shared_ptr<owt::conference::RemoteMixedStream> stream_ptr =
      std::static_pointer_cast<owt::conference::RemoteMixedStream>(
          [self nativeStream]);
  return [NSString stringForStdString:stream_ptr->Viewport()];
}
@end
