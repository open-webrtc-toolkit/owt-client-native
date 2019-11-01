// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <WebRTC/RTCMacros.h>
NS_ASSUME_NONNULL_BEGIN
/**
 @brief frame generator interface for audio
 @details Sample rate and channel numbers cannot be changed once the generator is
 created. Currently, only 16 bit little-endian PCM is supported.
*/
RTC_OBJC_EXPORT
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
RTC_OBJC_EXPORT
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
