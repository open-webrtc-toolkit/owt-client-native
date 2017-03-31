//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "talk/woogeen/sdk/base/objc/RTCStream+Internal.h"
#import "talk/woogeen/sdk/conference/objc/RTCRemoteMixedStream+Internal.h"
#import "talk/woogeen/sdk/conference/objc/RemoteMixedStreamObserverObjcImpl.h"
#import "webrtc/sdk/objc/Framework/Classes/NSString+StdString.h"

@implementation RTCRemoteMixedStream {
  NSArray* _supportedVideoFormats;
  std::vector<woogeen::conference::RemoteMixedStreamObserverObjcImpl*> _observers;
}

- (NSArray*)supportedVideoFormats {
  if (_supportedVideoFormats == nil) {
    _supportedVideoFormats = [[NSArray alloc] init];
  }
  return _supportedVideoFormats;
}

- (void)addObserver:(id<RTCRemoteMixedStreamObserver>)observer {
  auto ob =
      new woogeen::conference::RemoteMixedStreamObserverObjcImpl(observer);
  _observers.push_back(ob);
  std::shared_ptr<woogeen::conference::RemoteMixedStream> stream_ptr =
      std::static_pointer_cast<woogeen::conference::RemoteMixedStream>([self nativeStream]);
  stream_ptr->AddObserver(*ob);
}

- (NSString*)viewport {
  std::shared_ptr<woogeen::conference::RemoteMixedStream> stream_ptr =
      std::static_pointer_cast<woogeen::conference::RemoteMixedStream>(
          [self nativeStream]);
  return [NSString stringForStdString:stream_ptr->Viewport()];
}

@end

@implementation RTCRemoteMixedStream (Internal)

- (void)setSupportedVideoFormats:(NSArray*)supportedVideoFormats {
  _supportedVideoFormats = supportedVideoFormats;
}

@end
