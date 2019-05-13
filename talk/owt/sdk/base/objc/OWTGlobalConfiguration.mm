// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "talk/owt/sdk/include/objc/OWT/OWTGlobalConfiguration.h"
#import "talk/owt/sdk/include/cpp/owt/base/globalconfiguration.h"
#import "talk/owt/sdk/base/objc/FrameGeneratorObjcImpl.h"
@implementation OWTGlobalConfiguration
+ (void)setCustomizedAudioInputEnabled:(BOOL)enabled
                   audioFrameGenerator:
                       (id<RTCAudioFrameGeneratorProtocol>)audioFrameGenerator {
  if (!enabled || audioFrameGenerator == nil) {
    owt::base::GlobalConfiguration::SetCustomizedAudioInputEnabled(false,
                                                                       nullptr);
    return;
  }
  std::unique_ptr<owt::base::AudioFrameGeneratorInterface> generator(
      new owt::base::AudioFrameGeneratorObjcImpl(audioFrameGenerator));
  owt::base::GlobalConfiguration::SetCustomizedAudioInputEnabled(
      true, std::move(generator));
}
@end
