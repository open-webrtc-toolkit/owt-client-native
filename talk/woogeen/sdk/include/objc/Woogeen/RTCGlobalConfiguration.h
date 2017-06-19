/*
 * Copyright © 2016 Intel Corporation. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import <Foundation/Foundation.h>
#import "Woogeen/RTCFrameGeneratorProtocol.h"
#import <WebRTC/RTCMacros.h>

NS_ASSUME_NONNULL_BEGIN

RTC_EXPORT
@interface RTCGlobalConfiguration : NSObject

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
