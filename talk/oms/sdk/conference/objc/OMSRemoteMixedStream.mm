// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <Foundation/Foundation.h>
#import "talk/oms/sdk/base/objc/OMSStream+Private.h"
#import "talk/oms/sdk/conference/objc/RemoteMixedStreamObserverObjcImpl.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
@implementation OMSRemoteMixedStream {
  std::vector<oms::conference::RemoteMixedStreamObserverObjcImpl*> _observers;
}
@synthesize delegate;
- (NSString*)viewport {
  std::shared_ptr<oms::conference::RemoteMixedStream> stream_ptr =
      std::static_pointer_cast<oms::conference::RemoteMixedStream>(
          [self nativeStream]);
  return [NSString stringForStdString:stream_ptr->Viewport()];
}
@end
