//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "talk/ics/sdk/base/objc/RTCStream+Internal.h"
#import "talk/ics/sdk/conference/objc/RTCRemoteMixedStream+Internal.h"
#import "talk/ics/sdk/conference/objc/RemoteMixedStreamObserverObjcImpl.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"

@implementation RTCRemoteMixedStream {
  NSArray* _supportedVideoFormats;
  std::vector<ics::conference::RemoteMixedStreamObserverObjcImpl*> _observers;
}

- (NSArray*)supportedVideoFormats {
  if (_supportedVideoFormats == nil) {
    _supportedVideoFormats = [[NSArray alloc] init];
  }
  return _supportedVideoFormats;
}

- (void)addObserver:(id<RTCRemoteMixedStreamObserver>)observer {
  auto ob =
      new ics::conference::RemoteMixedStreamObserverObjcImpl(observer);
  _observers.push_back(ob);
  std::shared_ptr<ics::conference::RemoteMixedStream> stream_ptr =
      std::static_pointer_cast<ics::conference::RemoteMixedStream>([self nativeStream]);
  stream_ptr->AddObserver(*ob);
}

- (NSString*)viewport {
  std::shared_ptr<ics::conference::RemoteMixedStream> stream_ptr =
      std::static_pointer_cast<ics::conference::RemoteMixedStream>(
          [self nativeStream]);
  return [NSString stringForStdString:stream_ptr->Viewport()];
}

@end

@implementation RTCRemoteMixedStream (Internal)

- (void)setSupportedVideoFormats:(NSArray*)supportedVideoFormats {
  _supportedVideoFormats = supportedVideoFormats;
}

@end
