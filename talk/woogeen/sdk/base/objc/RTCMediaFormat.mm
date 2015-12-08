//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import "talk/woogeen/sdk/base/objc/RTCMediaFormat+Internal.h"

@implementation RTCVideoFormat {
  woogeen::VideoFormat* _videoFormat;
}

- (instancetype)init {
  self = [super init];
  woogeen::Resolution resolution(0, 0);
  _videoFormat = new woogeen::VideoFormat(resolution);
  return self;
}

- (void)dealloc {
  delete _videoFormat;
}

- (CGSize)resolution {
  return CGSizeMake(_videoFormat->resolution.width,
                    _videoFormat->resolution.height);
}

@end

@implementation RTCVideoFormat (Internal)

- (instancetype)initWithNativeVideoFormat:
    (const woogeen::VideoFormat&)videoFormat {
  self = [super init];
  woogeen::Resolution resolution(videoFormat.resolution.width,
                                 videoFormat.resolution.height);
  _videoFormat = new woogeen::VideoFormat(resolution);
  return self;
}

- (NSString*)description {
  return [NSString stringWithFormat:@"Resolution: %f x %f",
                                    [self resolution].width,
                                    [self resolution].height];
}

@end
