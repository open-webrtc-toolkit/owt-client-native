//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/conference/objc/ICSConferenceSubscribeOptions+Internal.h"

#include "webrtc/rtc_base/checks.h"

@implementation ICSConferenceSubscribeOptions {
  CGSize _resolution;
}

- (instancetype)init {
  if ((self = [super init])) {
    self.resolution = CGSizeMake(0, 0);
    self.videoQualityLevel = ICSConferenceVideoQualityLevelStandard;
  }
  return self;
}

- (CGSize)resolution {
  return _resolution;
}

- (void)setResolution:(CGSize)resolution {
  _resolution = resolution;
}

@end

@implementation ICSConferenceSubscribeOptions (Internal)

- (ics::conference::SubscribeOptions)nativeSubscribeOptions {
  using namespace ics::conference;
  ics::conference::SubscribeOptions options;
  options.resolution.width = _resolution.width;
  options.resolution.height = _resolution.height;
  switch([self videoQualityLevel]) {
      case ICSConferenceVideoQualityLevelStandard:
        options.video_quality_level =
            SubscribeOptions::VideoQualityLevel::kStandard;
        break;
      case ICSConferenceVideoQualityLevelBestQuality:
        options.video_quality_level =
            SubscribeOptions::VideoQualityLevel::kBestQuality;
        break;
      case ICSConferenceVideoQualityLevelBetterQuality:
        options.video_quality_level =
            SubscribeOptions::VideoQualityLevel::kBetterQuality;
        break;
      case ICSConferenceVideoQualityLevelBetterSpeed:
        options.video_quality_level =
            SubscribeOptions::VideoQualityLevel::kBetterSpeed;
        break;
      case ICSConferenceVideoQualityLevelBestSpeed:
        options.video_quality_level =
            SubscribeOptions::VideoQualityLevel::kBestSpeed;
        break;
      default:
        RTC_NOTREACHED();
        options.video_quality_level =
            SubscribeOptions::VideoQualityLevel::kStandard;
    }
  return options;
}

@end
