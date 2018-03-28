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
#import "ICS/ICSMediaFormat.h"

NS_ASSUME_NONNULL_BEGIN

@protocol ICSRemoteStreamDelegate;

/// This class represents a remote stream.
RTC_EXPORT
@interface ICSRemoteStream : ICSStream

/**
  Get the stream owner's ID.
  If will be an empty string if a stream is generated by MCU server.
*/
@property(nonatomic, strong, readonly) NSString* origin;
/**
  Get stream's ID
*/
@property(nonatomic, strong, readonly) NSString* streamId;


@property(nonatomic, strong, readonly) ICSPublicationSettings* settings;

/// Original settings for publishing this stream.
@property(nonatomic, strong, readonly) ICSSubscriptionCapabilities* capabilities;

/// Capabilities remote endpoint provides for subscription.
@property(nonatomic, weak, readwrite) id<ICSRemoteStreamDelegate> delegate;

@end

RTC_EXPORT
@protocol ICSRemoteStreamDelegate<NSObject>

/// Triggered when a stream is ended, or the stream is no longer available in
/// conference.
-(void)streamDidEnd:(ICSRemoteStream*)stream;

@end

NS_ASSUME_NONNULL_END