//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import "talk/woogeen/sdk/conference/objc/RTCConferenceSubscribeOptions+Internal.h"

@implementation RTCConferenceSubscribeOptions {
  CGSize _resolution;
}

- (CGSize)resolution {
  return _resolution;
}

- (void)setResolution:(CGSize)resolution {
  _resolution = resolution;
}

@end

@implementation RTCConferenceSubscribeOptions (Internal)

- (woogeen::conference::SubscribeOptions)nativeSubscribeOptions {
  woogeen::conference::SubscribeOptions options;
  options.resolution.width = _resolution.width;
  options.resolution.height = _resolution.height;
  return options;
}

@end
