//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "talk/ics/sdk/base/objc/ICSStream+Private.h"
#import "talk/ics/sdk/conference/objc/RemoteMixedStreamObserverObjcImpl.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"

@implementation ICSRemoteMixedStream {
  std::vector<ics::conference::RemoteMixedStreamObserverObjcImpl*> _observers;
}

@synthesize delegate;

- (NSString*)viewport {
  std::shared_ptr<ics::conference::RemoteMixedStream> stream_ptr =
      std::static_pointer_cast<ics::conference::RemoteMixedStream>(
          [self nativeStream]);
  return [NSString stringForStdString:stream_ptr->Viewport()];
}

@end
