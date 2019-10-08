// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <Foundation/Foundation.h>
#import "OWT/OWTFrameGeneratorProtocol.h"
#import <WebRTC/RTCMacros.h>
NS_ASSUME_NONNULL_BEGIN
RTC_OBJC_EXPORT
@interface OWTGlobalConfiguration : NSObject
/**
 @brief Sets customized audio input enabled or not.
 @details When it is enabled, SDK will fetch audio frames from
 |audioFrameGenerator| instead of hardware audio devices, like mic.
 @param enabled Customized audio input enabled or not.
 @param audioFrameGenerator An implementation which feeds audio frames to SDK.
 If |enabled| is NO, generator will be ignored. It cannot be nil if customized
 audio input is enabled. If generator is nil, customized audio input will be
 disabled.
 */
+ (void)setCustomizedAudioInputEnabled:(BOOL)enabled
                   audioFrameGenerator:
                       (nullable id<RTCAudioFrameGeneratorProtocol>)
                           audioFrameGenerator;
@end
NS_ASSUME_NONNULL_END
