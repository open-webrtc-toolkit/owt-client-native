//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#import "talk/woogeen/sdk/include/objc/Woogeen/RTCGlobalConfiguration.h"
#import "talk/woogeen/sdk/include/cpp/woogeen/base/globalconfiguration.h"
#import "talk/woogeen/sdk/base/objc/FrameGeneratorObjcImpl.h"

@implementation RTCGlobalConfiguration

+ (void)setCustomizedAudioInputEnabled:(BOOL)enabled
                   audioFrameGenerator:
                       (id<RTCAudioFrameGeneratorProtocol>)audioFrameGenerator {
  if (!enabled || audioFrameGenerator == nil) {
    woogeen::base::GlobalConfiguration::SetCustomizedAudioInputEnabled(false,
                                                                       nullptr);
  }
  std::unique_ptr<woogeen::base::AudioFrameGeneratorInterface> generator(
      new woogeen::base::AudioFrameGeneratorObjcImpl(audioFrameGenerator));
  woogeen::base::GlobalConfiguration::SetCustomizedAudioInputEnabled(
      true, std::move(generator));
}

@end
