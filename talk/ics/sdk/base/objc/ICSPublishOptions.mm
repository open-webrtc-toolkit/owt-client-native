//
//  Copyright (c) 2018 Intel Corporation. All rights reserved.
//

#include "talk/ics/sdk/include/cpp/ics/base/options.h"

#import "talk/ics/sdk/base/objc/ICSPublishOptions+Private.h"
#import "talk/ics/sdk/base/objc/ICSMediaFormat+Private.h"

@implementation ICSPublishOptions

@dynamic nativePublishOptions;

- (std::shared_ptr<ics::base::PublishOptions>)nativePublishOptions {
  std::shared_ptr<ics::base::PublishOptions> options =
      std::shared_ptr<ics::base::PublishOptions>(
          new ics::base::PublishOptions());
  options->audio =
      std::vector<ics::base::AudioEncodingParameters>();
  for (ICSAudioEncodingParameters* parameter in _audio) {
    options->audio.push_back([parameter nativeAudioEncodingParameters]);
  }
  options->video =
      std::vector<ics::base::VideoEncodingParameters>();
  for (ICSVideoEncodingParameters* parameter in _video) {
    options->video.push_back([parameter nativeVideoEncodingParameters]);
  }
  return options;
}

@end 
