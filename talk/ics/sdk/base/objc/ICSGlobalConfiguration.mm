//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/include/objc/ICS/ICSGlobalConfiguration.h"
#import "talk/ics/sdk/include/cpp/ics/base/globalconfiguration.h"
#import "talk/ics/sdk/base/objc/FrameGeneratorObjcImpl.h"

@implementation ICSGlobalConfiguration

+ (void)setCustomizedAudioInputEnabled:(BOOL)enabled
                   audioFrameGenerator:
                       (id<RTCAudioFrameGeneratorProtocol>)audioFrameGenerator {
  if (!enabled || audioFrameGenerator == nil) {
    ics::base::GlobalConfiguration::SetCustomizedAudioInputEnabled(false,
                                                                       nullptr);
  }
  std::unique_ptr<ics::base::AudioFrameGeneratorInterface> generator(
      new ics::base::AudioFrameGeneratorObjcImpl(audioFrameGenerator));
  ics::base::GlobalConfiguration::SetCustomizedAudioInputEnabled(
      true, std::move(generator));
}

@end
