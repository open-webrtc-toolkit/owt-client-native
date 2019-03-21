// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "talk/owt/sdk/include/cpp/owt/base/options.h"
#import "talk/owt/sdk/base/objc/OWTPublishOptions+Private.h"
#import "talk/owt/sdk/base/objc/OWTMediaFormat+Private.h"
@implementation OWTPublishOptions
@dynamic nativePublishOptions;
- (std::shared_ptr<owt::base::PublishOptions>)nativePublishOptions {
  std::shared_ptr<owt::base::PublishOptions> options =
      std::shared_ptr<owt::base::PublishOptions>(
          new owt::base::PublishOptions());
  options->audio =
      std::vector<owt::base::AudioEncodingParameters>();
  for (OWTAudioEncodingParameters* parameter in _audio) {
    options->audio.push_back([parameter nativeAudioEncodingParameters]);
  }
  options->video =
      std::vector<owt::base::VideoEncodingParameters>();
  for (OWTVideoEncodingParameters* parameter in _video) {
    options->video.push_back([parameter nativeVideoEncodingParameters]);
  }
  return options;
}
@end
