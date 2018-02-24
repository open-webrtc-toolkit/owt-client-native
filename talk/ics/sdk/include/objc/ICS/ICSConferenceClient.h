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

#ifndef ICS_CONFERENCE_OBJC_ICSCONFERENCECLIENT_H_
#define ICS_CONFERENCE_OBJC_ICSCONFERENCECLIENT_H_

#import <WebRTC/RTCMacros.h>
#import "ICS/ICSConferenceClientConfiguration.h"
#import "ICS/ICSConferencePublication.h"
#import "ICS/ICSConferenceSubscription.h"
#import "ICS/ICSConferenceParticipant.h"
#import "ICS/ICSConferenceInfo.h"
#import "ICS/ICSConnectionStats.h"
#import "ICS/ICSLocalStream.h"
#import "ICS/ICSRemoteMixedStream.h"
#import "ICS/ICSRemoteStream.h"

NS_ASSUME_NONNULL_BEGIN

@protocol ICSConferenceClientDelegate;

/// An asynchronous class for app to communicate with a conference in MCU
RTC_EXPORT
@interface ICSConferenceClient : NSObject

/**
  @brief Initialize a ICSConferenceClient with configuration
  @param config Configuration for creating the ICSConferenceClient.
*/
- (instancetype)initWithConfiguration:(ICSConferenceClientConfiguration*)config;
/**
  @brief Connect to the specified room to join a conference.
  @param token Includes the room info which is encrypted.
  @param onSuccess Success callback with the conference info.
*/
- (void)joinWithToken:(NSString*)token
            onSuccess:(nullable void (^)(ICSConferenceInfo*))onSuccess
            onFailure:(nullable void (^)(NSError*))onFailure;
/**
  @brief Publish the stream to the current room.
  @param stream The stream to be published.
*/
- (void)publish:(ICSLocalStream*)stream
      onSuccess:(nullable void (^)(ICSConferencePublication*))onSuccess
      onFailure:(nullable void (^)(NSError*))onFailure;
/**
  @brief Subscribe a stream from the current room.
  @param stream The remote stream to be subscribed.
  @param onSuccess Success callback with a stream that contains media stream.
*/
- (void)subscribe:(ICSRemoteStream*)stream
        onSuccess:(nullable void (^)(ICSConferenceSubscription*))onSuccess
        onFailure:(nullable void (^)(NSError*))onFailure;
/**
  @brief Subscribe a stream from the current room.
  @param stream The remote stream to be subscribed.
  @param options Options for subscribing the stream.
  @param onSuccess Success callback with a stream that contains media stream.
*/
- (void)subscribe:(ICSRemoteStream*)stream
      withOptions:(ICSConferenceSubscriptionOptions*)options
        onSuccess:(nullable void (^)(ICSConferenceSubscription*))onSuccess
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
  @brief Leave current conference.
*/
- (void)leaveWithOnSuccess:(nullable void (^)())onSuccess
                 onFailure:(nullable void (^)(NSError*))onFailure;

@property(nonatomic, weak) id<ICSConferenceClientDelegate> delegate;

@end

/// Delegate for ICSConferenceClient.
RTC_EXPORT
@protocol ICSConferenceClientDelegate<NSObject>

@optional
/**
  @brief Triggers when client is disconnected from conference server.
*/
- (void)conferenceClientDidDisconnect:(ICSConferenceClient*)client;
/**
  @brief Triggers when a stream is added.
  @param stream The stream which is added.
*/
- (void)conferenceClient:(ICSConferenceClient*)client
            didAddStream:(ICSRemoteStream*)stream;
/**
  @brief Triggers when a message is received.
  @param senderId Sender's ID.
  @param message Message received.
*/
- (void)conferenceClient:(ICSConferenceClient*)client
       didReceiveMessage:(NSString*)message
                    from:(NSString*)senderId;
/**
  @brief Triggers when a user joined conference.
  @param user The user joined.
*/
- (void)conferenceClient:(ICSConferenceClient*)client
       didAddParticipant:(ICSConferenceParticipant*)user;

@end

NS_ASSUME_NONNULL_END

#endif  // ICS_CONFERENCE_OBJC_ICSCONFERENCECLIENT_H_
