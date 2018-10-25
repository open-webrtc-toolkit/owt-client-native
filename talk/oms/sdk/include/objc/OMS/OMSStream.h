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
