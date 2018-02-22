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

#import "ICS/ICSStream.h"
#import "WebRTC/WebRTC.h"

NS_ASSUME_NONNULL_BEGIN

@class ICSStreamConstraints;

/// This class represent a local stream.
RTC_EXPORT
@interface ICSLocalStream : ICSStream

/**
  Create an ICSLocalStream from given RTCMediaStream.
  @param source Information about stream's source.
*/
- (instancetype)initWithMediaStream:(RTCMediaStream*)mediaStream
                         sourceInfo:(ICSStreamSourceInfo*)source;

/**
  Create an ICSLocalStream from mic and camera with given constraints.
  @param constraints Constraints for creating the stream. The stream will not be
  impacted if changing constraints after it is created.
  @return On success, an ICSLocalStream object. If nil, the outError parameter
  contains an NSError instance describing the problem.
*/
- (instancetype)initWithConstratins:(ICSStreamConstraints*)constraints
                              error:(NSError**)outError;

/**
  @brief Set a user-defined attribute map.
  @details Remote user can get attribute map by calling setAttributes:. P2P mode
  does not support setting attributes.
*/
- (void)setAttributes:(NSDictionary<NSString*, NSString*>*)attributes;

@end

NS_ASSUME_NONNULL_END
