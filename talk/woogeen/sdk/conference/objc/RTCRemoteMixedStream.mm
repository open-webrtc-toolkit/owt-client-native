//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "talk/woogeen/sdk/base/objc/RTCStream+Internal.h"
#import "talk/woogeen/sdk/conference/objc/RTCRemoteMixedStream+Internal.h"

@implementation RTCRemoteMixedStream {
  NSArray* _supportedVideoFormats;
}

- (NSArray*)supportedVideoFormats {
  if (_supportedVideoFormats == nil) {
    _supportedVideoFormats = [[NSArray alloc] init];
  }
  return _supportedVideoFormats;
}

@end

@implementation RTCRemoteMixedStream (Internal)

- (void)setSupportedVideoFormats:(NSArray*)supportedVideoFormats {
  _supportedVideoFormats = supportedVideoFormats;
}

@end
