//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#import "talk/woogeen/sdk/conference/objc/RTCConferenceSubscribeOptions+Internal.h"

#include "webrtc/base/checks.h"

@implementation RTCConferenceSubscribeOptions {
  CGSize _resolution;
}

- (instancetype)init {
  if ((self = [super init])) {
    self.resolution = CGSizeMake(0, 0);
    self.videoQualityLevel = RTCConferenceVideoQualityLevelStandard;
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

@implementation RTCConferenceSubscribeOptions (Internal)

- (woogeen::conference::SubscribeOptions)nativeSubscribeOptions {
  using namespace woogeen::conference;
  woogeen::conference::SubscribeOptions options;
  options.resolution.width = _resolution.width;
  options.resolution.height = _resolution.height;
  switch([self videoQualityLevel]) {
      case RTCConferenceVideoQualityLevelStandard:
        options.video_quality_level =
            SubscribeOptions::VideoQualityLevel::kStandard;
        break;
      case RTCConferenceVideoQualityLevelBestQuality:
        options.video_quality_level =
            SubscribeOptions::VideoQualityLevel::kBestQuality;
        break;
      case RTCConferenceVideoQualityLevelBetterQuality:
        options.video_quality_level =
            SubscribeOptions::VideoQualityLevel::kBetterQuality;
        break;
      case RTCConferenceVideoQualityLevelBetterSpeed:
        options.video_quality_level =
            SubscribeOptions::VideoQualityLevel::kBetterSpeed;
        break;
      case RTCConferenceVideoQualityLevelBestSpeed:
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
