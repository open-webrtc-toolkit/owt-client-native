//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "talk/ics/sdk/base/objc/ICSStream+Private.h"
#import "talk/ics/sdk/conference/objc/ICSRemoteMixedStream+Internal.h"
#import "talk/ics/sdk/conference/objc/RemoteMixedStreamObserverObjcImpl.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"

@implementation ICSRemoteMixedStream {
  NSArray* _supportedVideoFormats;
  std::vector<ics::conference::RemoteMixedStreamObserverObjcImpl*> _observers;
}

@synthesize delegate;

- (NSArray*)supportedVideoFormats {
  if (_supportedVideoFormats == nil) {
    _supportedVideoFormats = [[NSArray alloc] init];
  }
  return _supportedVideoFormats;
}


- (NSString*)viewport {
  std::shared_ptr<ics::conference::RemoteMixedStream> stream_ptr =
      std::static_pointer_cast<ics::conference::RemoteMixedStream>(
          [self nativeStream]);
  return [NSString stringForStdString:stream_ptr->Viewport()];
}

@end

@implementation ICSRemoteMixedStream (Internal)

- (void)setSupportedVideoFormats:(NSArray*)supportedVideoFormats {
  _supportedVideoFormats = supportedVideoFormats;
}

@end
