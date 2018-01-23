//
//  Copyright (c) 2018 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/conference/objc/ICSConferencePublication+Private.h"

@implementation ICSConferencePublication {
  std::shared_ptr<ics::conference::ConferencePublication> _nativePublication;
}

- (instancetype)initWithNativePublication:
    (std::shared_ptr<ics::conference::ConferencePublication>)nativePublication {
  self = [super init];
  _nativePublication = nativePublication;
  return self;
}

- (void)stop {
  _nativePublication->Stop(nullptr, nullptr);
}

@end
