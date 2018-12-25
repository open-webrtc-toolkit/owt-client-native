// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "talk/oms/sdk/include/objc/OMS/OMSGlobalConfiguration.h"
#import "talk/oms/sdk/include/cpp/oms/base/globalconfiguration.h"
#import "talk/oms/sdk/base/objc/FrameGeneratorObjcImpl.h"
@implementation OMSGlobalConfiguration
+ (void)setCustomizedAudioInputEnabled:(BOOL)enabled
                   audioFrameGenerator:
                       (id<RTCAudioFrameGeneratorProtocol>)audioFrameGenerator {
  if (!enabled || audioFrameGenerator == nil) {
    oms::base::GlobalConfiguration::SetCustomizedAudioInputEnabled(false,
                                                                       nullptr);
  }
  std::unique_ptr<oms::base::AudioFrameGeneratorInterface> generator(
      new oms::base::AudioFrameGeneratorObjcImpl(audioFrameGenerator));
  oms::base::GlobalConfiguration::SetCustomizedAudioInputEnabled(
      true, std::move(generator));
}
@end
