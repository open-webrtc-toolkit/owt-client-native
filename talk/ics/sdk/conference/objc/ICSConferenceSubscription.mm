//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/conference/objc/ICSConferenceSubscription+Private.h"

#include "webrtc/rtc_base/checks.h"

@implementation ICSConferenceSubscription {
  std::shared_ptr<ics::conference::ConferenceSubscription> _nativeSubscription;
}

- (instancetype)initWithNativeSubscription:
    (std::shared_ptr<ics::conference::ConferenceSubscription>)
        nativeSubscription {
  self = [super init];
  _nativeSubscription = nativeSubscription;
  return self;
}

- (void)stop {
  _nativeSubscription->Stop(nullptr, nullptr);
}

@end

@implementation ICSConferenceSubscriptionOptions {
  CGSize _resolution;
}

- (instancetype)init {
  if ((self = [super init])) {
    self.resolution = CGSizeMake(0, 0);
  }
  return self;
}

- (CGSize)resolution {
  return _resolution;
}

- (void)setResolution:(CGSize)resolution {
  _resolution = resolution;
}

- (ics::conference::SubscriptionOptions)nativeSubscriptionOptions {
  using namespace ics::conference;
  ics::conference::SubscriptionOptions options;
  options.video.resolution.width = _resolution.width;
  options.video.resolution.height = _resolution.height;
  return options;
}

@end
