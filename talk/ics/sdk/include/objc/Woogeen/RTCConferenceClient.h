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

#ifndef WOOGEEN_CONFERENCE_OBJC_RTCCONFERENCECLIENT_H_
#define WOOGEEN_CONFERENCE_OBJC_RTCCONFERENCECLIENT_H_

#import <WebRTC/RTCMacros.h>
#import "Woogeen/RTCLocalStream.h"
#import "Woogeen/RTCRemoteMixedStream.h"
#import "Woogeen/RTCRemoteStream.h"
#import "Woogeen/RTCConferenceClientObserver.h"
#import "Woogeen/RTCConferenceClientConfiguration.h"
#import "Woogeen/RTCConferenceSubscribeOptions.h"
#import "Woogeen/RTCConferenceUser.h"
#import "Woogeen/RTCConnectionStats.h"
#import "Woogeen/RTCExternalOutput.h"

NS_ASSUME_NONNULL_BEGIN

/// An asynchronous class for app to communicate with a conference in MCU
RTC_EXPORT
@interface RTCConferenceClient : NSObject

/**
  @brief Initialize a RTCConferenceClient with configuration
  @param config Configuration for creating the RTCConferenceClient.
*/
- (instancetype)initWithConfiguration:(RTCConferenceClientConfiguration*)config;
/**
  @brief Connect to the specified room to join a conference.
  @param token Includes the room info which is encrypted.
*/
- (void)joinWithToken:(NSString*)token
            onSuccess:(nullable void (^)(RTCConferenceUser*))onSuccess
            onFailure:(nullable void (^)(NSError*))onFailure;
/**
  @brief Publish the stream to the current room.
  @param stream The stream to be published.
*/
- (void)publish:(RTCLocalStream*)stream
      onSuccess:(nullable void (^)())onSuccess
      onFailure:(nullable void (^)(NSError*))onFailure;
/**
  @brief Subscribe a stream from the current room.
  @param stream The remote stream to be subscribed.
  @param onSuccess Success callback with a stream that contains media stream.
*/
- (void)subscribe:(RTCRemoteStream*)stream
        onSuccess:(nullable void (^)(RTCRemoteStream*))onSuccess
        onFailure:(nullable void (^)(NSError*))onFailure;
/**
  @brief Subscribe a stream from the current room.
  @param stream The remote stream to be subscribed.
  @param options Options for subscribing the stream.
  @param onSuccess Success callback with a stream that contains media stream.
*/
- (void)subscribe:(RTCRemoteStream*)stream
        withOptions:(RTCConferenceSubscribeOptions*)options
        onSuccess:(nullable void (^)(RTCRemoteStream*))onSuccess
        onFailure:(nullable void (^)(NSError*))onFailure;
/**
  @brief Send message to all participants in the conference.
  @param message The message to be sent.
*/
- (void)send:(NSString*)message
   onSuccess:(nullable void (^)())onSuccess
   onFailure:(nullable void (^)(NSError*))onFailure;
/**
  @brief Send message to specific participant in the conference.
  @param message The message to be sent.
  @param to The user who receives this message.
*/
- (void)send:(NSString*)message
          to:(NSString*)receiver
   onSuccess:(nullable void (^)())onSuccess
   onFailure:(nullable void (^)(NSError*))onFailure;

/**
  @brief Get a stream's connection statistics
*/
- (void)getConnectionStatsForStream:(RTCStream*)stream
                          onSuccess:
                              (nullable void (^)(RTCConnectionStats*))onSuccess
                          onFailure:(nullable void (^)(NSError*))onFailure;

/**
  @brief Leave current conference.
*/
- (void)leaveWithOnSuccess:(nullable void (^)())onSuccess
                 onFailure:(nullable void (^)(NSError*))onFailure;
/**
  @brief Add an observer for ConferenceClient.
*/
- (void)addObserver:(id<RTCConferenceClientObserver>)observer;
/// @cond
/**
  @brief Remove an observer from the ConferenceClient.
*/
- (void)removeObserver:(id<RTCConferenceClientObserver>)observer;
/// @endcond

@end

NS_ASSUME_NONNULL_END

#endif  // WOOGEEN_CONFERENCE_OBJC_RTCCONFERENCECLIENT_H_
