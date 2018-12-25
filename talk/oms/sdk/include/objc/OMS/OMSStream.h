// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <Foundation/Foundation.h>
#import <WebRTC/RTCMacros.h>
#import <WebRTC/RTCVideoRenderer.h>
NS_ASSUME_NONNULL_BEGIN
@class OMSStreamSourceInfo;
@class RTCMediaStream;
/// Base class of all streams in the SDK
RTC_EXPORT
@interface OMSStream : NSObject
// Writable because mediaStream is subscribed after OMSRemoteStream is created in conference mode.
@property(nonatomic, strong, readwrite) RTCMediaStream* mediaStream;
@property(nonatomic, strong, readonly) OMSStreamSourceInfo* source;
- (instancetype)init /*NS_UNAVAILABLE*/;
/**
  @brief Attach the stream's first video track to a renderer.
  @details The render doesn't retain this stream. Using this method is not
  recommended. It will be removed in the future. please use
  [RTCVideoTrack addRenderer] instead.
 */
- (void)attach:(NSObject<RTCVideoRenderer>*)renderer;
/**
  @brief Returns a user-defined attribute dictionary.
  @details These attributes are defined by publisher. P2P mode always return
  empty dictionary because it is not supported yet.
*/
- (NSDictionary<NSString*, NSString*>*)attributes;
@end
NS_ASSUME_NONNULL_END
