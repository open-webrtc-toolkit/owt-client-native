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

#ifndef OMS_CONFERENCE_OBJC_OMSCONFERENCECLIENT_H_
#define OMS_CONFERENCE_OBJC_OMSCONFERENCECLIENT_H_

#import <WebRTC/RTCMacros.h>
#import "OMS/OMSConferenceClientConfiguration.h"
#import "OMS/OMSConferencePublication.h"
#import "OMS/OMSConferenceSubscription.h"
#import "OMS/OMSConferenceParticipant.h"
#import "OMS/OMSConferenceInfo.h"
#import "OMS/OMSLocalStream.h"
#import "OMS/OMSRemoteMixedStream.h"
#import "OMS/OMSRemoteStream.h"
#import "OMS/OMSPublishOptions.h"

NS_ASSUME_NONNULL_BEGIN

@protocol OMSConferenceClientDelegate;

/// An asynchronous class for app to communicate with a conference in MCU
RTC_EXPORT
@interface OMSConferenceClient : NSObject

/**
  @brief Initialize a OMSConferenceClient with configuration
  @param config Configuration for creating the OMSConferenceClient.
*/
- (instancetype)initWithConfiguration:(OMSConferenceClientConfiguration*)config;
/**
  @brief Connect to the specified room to join a conference.
  @param token Includes the room info which is encrypted.
  @param onSuccess Success callback with the conference info.
*/
- (void)joinWithToken:(NSString*)token
            onSuccess:(nullable void (^)(OMSConferenceInfo*))onSuccess
            onFailure:(nullable void (^)(NSError*))onFailure;

/**
  @brief Publish the stream to the current room.
  @param stream The stream to be published.
*/
- (void)publish:(OMSLocalStream*)stream
    withOptions:(nullable OMSPublishOptions*)options
      onSuccess:(nullable void (^)(OMSConferencePublication*))onSuccess
      onFailure:(nullable void (^)(NSError*))onFailure;
/**
  @brief Subscribe a stream from the current room.
  @param stream The remote stream to be subscribed.
  @param options Options for subscribing the stream.
  @param onSuccess Success callback with a stream that contains media stream.
*/
- (void)subscribe:(OMSRemoteStream*)stream
      withOptions:(nullable OMSConferenceSubscribeOptions*)options
        onSuccess:(nullable void (^)(OMSConferenceSubscription*))onSuccess
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

@property(nonatomic, weak) id<OMSConferenceClientDelegate> delegate;

@end

/// Delegate for OMSConferenceClient.
RTC_EXPORT
@protocol OMSConferenceClientDelegate<NSObject>

@optional
/**
  @brief Triggers when client is disconnected from conference server.
*/
- (void)conferenceClientDidDisconnect:(OMSConferenceClient*)client;
/**
  @brief Triggers when a stream is added.
  @param stream The stream which is added.
*/
- (void)conferenceClient:(OMSConferenceClient*)client
            didAddStream:(OMSRemoteStream*)stream;
/**
  @brief Triggers when a message is received.
  @param senderId Sender's ID.
  @param message Message received.
  @param targetType "all" if broadcast message. "me"
  if the message is sent only to current conference client.
*/
- (void)conferenceClient:(OMSConferenceClient*)client
       didReceiveMessage:(NSString*)message
                    from:(NSString*)senderId
                    to:(NSString*)targetType;
/**
  @brief Triggers when a user joined conference.
  @param user The user joined.
*/
- (void)conferenceClient:(OMSConferenceClient*)client
       didAddParticipant:(OMSConferenceParticipant*)user;

@end

NS_ASSUME_NONNULL_END

#endif  // OMS_CONFERENCE_OBJC_OMSCONFERENCECLIENT_H_
