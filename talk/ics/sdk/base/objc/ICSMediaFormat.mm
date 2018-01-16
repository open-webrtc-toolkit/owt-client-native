//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/base/objc/ICSMediaFormat+Internal.h"

@implementation ICSVideoFormat {
  ics::base::VideoFormat* _videoFormat;
}

- (instancetype)init {
  self = [super init];
  ics::base::Resolution resolution(0, 0);
  _videoFormat = new ics::base::VideoFormat(resolution);
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

@implementation ICSVideoFormat (Internal)

- (instancetype)initWithNativeVideoFormat:
    (const ics::base::VideoFormat&)videoFormat {
  self = [super init];
  ics::base::Resolution resolution(videoFormat.resolution.width,
                                 videoFormat.resolution.height);
  _videoFormat = new ics::base::VideoFormat(resolution);
  return self;
}

- (NSString*)description {
  return [NSString stringWithFormat:@"Resolution: %f x %f",
                                    [self resolution].width,
                                    [self resolution].height];
}

@end
