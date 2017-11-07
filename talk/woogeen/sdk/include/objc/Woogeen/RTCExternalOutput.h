/*
 * Copyright © 2017 Intel Corporation. All Rights Reserved.
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

#ifndef WOOGEEN_CONFERENCE_OBJC_RTCEXTERNALOUTPUT_H_
#define WOOGEEN_CONFERENCE_OBJC_RTCEXTERNALOUTPUT_H_

#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>
#import <WebRTC/RTCMacros.h>
#import <Woogeen/RTCStream.h>

NS_ASSUME_NONNULL_BEGIN

/// Options for external audio output.
RTC_EXPORT
@interface RTCExternalAudioOutputOptions : NSObject
/**
  @brief Indicates whether audio will be outputted.
  @detail If it is false, all other audio options will be ignored.
*/
@property(nonatomic, assign) BOOL enabled;

@end

RTC_EXPORT
@interface RTCExternalVideoOutputOptions : NSObject
/**
  @brief Indicates whether video will be outputted.
  @detail If it is false, all other video options will be ignored.
*/
@property(nonatomic, assign) BOOL enabled;
/// Resolution of output stream.
@property(nonatomic, assign) CGSize resolution;

@end

RTC_EXPORT
/// Options for external output.
@interface RTCExternalOutputOptions : NSObject
/// The stream that will be streamed to a specific target.
@property(nonatomic, retain) RTCStream* stream;
/**
  @brief Target URL.
  @detail If |url|'s scheme is file, the stream will be recorded in MCU. You can
  create a file URL with any path to indicate the output is recording. Path is
  ignored currently.
  */
@property(nonatomic, retain) NSURL* url;
/// Options for external audio output.
@property(nonatomic, retain) RTCExternalAudioOutputOptions* audioOptions;
/// Options for external video output.
@property(nonatomic, retain) RTCExternalVideoOutputOptions* videoOptions;

@end

RTC_EXPORT
/// Ack for adding and updating external output.
@interface RTCExternalOutputAck : NSObject
/// External output streaming URL or recorder ID.
// TODO: It should be changed to NSURL.
@property(nonatomic, retain, readonly) NSString* url;

@end

NS_ASSUME_NONNULL_END

#endif  // WOOGEEN_CONFERENCE_OBJC_RTCEXTERNALOUTPUT_H_
