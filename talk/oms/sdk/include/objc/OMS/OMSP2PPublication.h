/*
 * Copyright © 2018 Intel Corporation. All Rights Reserved.
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

NS_ASSUME_NONNULL_BEGIN

@class OMSP2PPublication;

RTC_EXPORT
@protocol OMSP2PPublicationDelegate<NSObject>

@optional

/// Publication is ended.
- (void)publicationDidEnd:(OMSP2PPublication*)publication;

@end

/**
  @brief Publication represents a sender for publishing a stream.
  @details It handles the actions on a LocalStream published to a remote endpoint.
*/
RTC_EXPORT
@interface OMSP2PPublication : NSObject

- (instancetype)init NS_UNAVAILABLE;

/// Stop certain publication. Once a subscription is stopped, it cannot be recovered.
- (void)stop;

/// Get stats of underlying PeerConnection.
- (void)stats:(void (^)(NSArray<RTCLegacyStatsReport*>* stats))onSuccess
    onFailure:(nullable void (^)(NSError*))onFailure;

@property(nonatomic, weak) id<OMSP2PPublicationDelegate> delegate;

@end

NS_ASSUME_NONNULL_END
