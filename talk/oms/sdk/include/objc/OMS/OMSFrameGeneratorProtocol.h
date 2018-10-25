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
#import <UIKit/UIKit.h>
#import <WebRTC/RTCMacros.h>

NS_ASSUME_NONNULL_BEGIN

/**
 @brief frame generator interface for audio
 @details Sample rate and channel numbers cannot be changed once the generator is
 created. Currently, only 16 bit little-endian PCM is supported.
*/
RTC_EXPORT
@protocol RTCAudioFrameGeneratorProtocol<NSObject>

/**
 @brief Generate frames for next 10ms.
 @param buffer Points to the start address for frame data. The memory is
 allocated and owned by SDK. Implementations should fill frame data to the
 memory starts from |buffer|.
 @param capacity Buffer's capacity. It will be equal or greater to expected
 frame buffer size.
 @return The size of actually frame buffer size.
 */
- (NSUInteger)framesForNext10Ms:(uint8_t*)buffer capacity:(const NSUInteger)capacity;
/// Get sample rate for frames generated.
- (NSUInteger)sampleRate;
/// Get numbers of channel for frames generated.
- (NSUInteger)channelNumber;
@end

/**
 @brief Protocol for video frame generators

 RTCLocalCustomizedStream pulls video frames from an object implements this
 protocol. Height, width and frame rate cannot be changed once generator is
 created.
 */
RTC_EXPORT
@protocol RTCVideoFrameGeneratorProtocol<NSObject>
/**
 @brief Generate next video frame
 @param buffer Points to the start address for frame data. The memory is
 allocated and owned by SDK. Implementations should fill frame data to the
 memory starts from |buffer|.
 @param capacity Buffer's capacity. It will be equal or greater to expected
 frame buffer size.
 @return The size of actually frame buffer size.
*/
- (NSUInteger)nextFrame:(uint8_t*)buffer capacity:(const NSUInteger)capacity;
/**
 @brief frame resolution
*/
- (CGSize)resolution;
/**
 @brief frame rate, unit: fps
*/
- (NSUInteger)frameRate;
@end

NS_ASSUME_NONNULL_END
