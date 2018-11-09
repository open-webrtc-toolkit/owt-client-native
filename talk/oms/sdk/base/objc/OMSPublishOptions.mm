//
//  Copyright (c) 2018 Intel Corporation. All rights reserved.
//
#include "talk/oms/sdk/include/cpp/oms/base/options.h"
#import "talk/oms/sdk/base/objc/OMSPublishOptions+Private.h"
#import "talk/oms/sdk/base/objc/OMSMediaFormat+Private.h"
@implementation OMSPublishOptions
@dynamic nativePublishOptions;
- (std::shared_ptr<oms::base::PublishOptions>)nativePublishOptions {
  std::shared_ptr<oms::base::PublishOptions> options =
      std::shared_ptr<oms::base::PublishOptions>(
          new oms::base::PublishOptions());
  options->audio =
      std::vector<oms::base::AudioEncodingParameters>();
  for (OMSAudioEncodingParameters* parameter in _audio) {
    options->audio.push_back([parameter nativeAudioEncodingParameters]);
  }
  options->video =
      std::vector<oms::base::VideoEncodingParameters>();
  for (OMSVideoEncodingParameters* parameter in _video) {
    options->video.push_back([parameter nativeVideoEncodingParameters]);
  }
  return options;
}
@end 
