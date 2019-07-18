// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_CONFERENCE_OBJC_OWTCONFERENCECLIENT_H_
#define OWT_CONFERENCE_OBJC_OWTCONFERENCECLIENT_H_
#import <WebRTC/RTCMacros.h>
#import "OWT/OWTConferenceClientConfiguration.h"
#import "OWT/OWTConferencePublication.h"
#import "OWT/OWTConferenceSubscription.h"
#import "OWT/OWTConferenceParticipant.h"
#import "OWT/OWTConferenceInfo.h"
#import "OWT/OWTLocalStream.h"
#import "OWT/OWTRemoteMixedStream.h"
#import "OWT/OWTRemoteStream.h"
#import "OWT/OWTPublishOptions.h"
NS_ASSUME_NONNULL_BEGIN
@protocol OWTConferenceClientDelegate;
/// An asynchronous class for app to communicate with a conference in MCU
RTC_OBJC_EXPORT
@interface OWTConferenceClient : NSObject
/**
  @brief Initialize a OWTConferenceClient with configuration
  @param config Configuration for creating the OWTConferenceClient.
*/
- (instancetype)initWithConfiguration:(OWTConferenceClientConfiguration*)config;
/**
  @brief Connect to the specified room to join a conference.
  @param token Includes the room info which is encrypted.
  @param onSuccess Success callback with the conference info.
*/
- (void)joinWithToken:(NSString*)token
            onSuccess:(nullable void (^)(OWTConferenceInfo*))onSuccess
            onFailure:(nullable void (^)(NSError*))onFailure;
/**
  @brief Publish the stream to the current room.
  @param stream The stream to be published.
*/
- (void)publish:(OWTLocalStream*)stream
    withOptions:(nullable OWTPublishOptions*)options
      onSuccess:(nullable void (^)(OWTConferencePublication*))onSuccess
      onFailure:(nullable void (^)(NSError*))onFailure;
/**
  @brief Subscribe a stream from the current room.
  @param stream The remote stream to be subscribed.
  @param options Options for subscribing the stream.
  @param onSuccess Success callback with a stream that contains media stream.
*/
- (void)subscribe:(OWTRemoteStream*)stream
      withOptions:(nullable OWTConferenceSubscribeOptions*)options
        onSuccess:(nullable void (^)(OWTConferenceSubscription*))onSuccess
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
@property(nonatomic, weak) id<OWTConferenceClientDelegate> delegate;
@end
/// Delegate for OWTConferenceClient.
RTC_OBJC_EXPORT
@protocol OWTConferenceClientDelegate<NSObject>
@optional
/**
  @brief Triggers when client is disconnected from conference server.
*/
- (void)conferenceClientDidDisconnect:(OWTConferenceClient*)client;
/**
  @brief Triggers when a stream is added.
  @param stream The stream which is added.
*/
- (void)conferenceClient:(OWTConferenceClient*)client
            didAddStream:(OWTRemoteStream*)stream;
/**
  @brief Triggers when a message is received.
  @param senderId Sender's ID.
  @param message Message received.
  @param targetType "all" if broadcast message. "me"
  if the message is sent only to current conference client.
*/
- (void)conferenceClient:(OWTConferenceClient*)client
       didReceiveMessage:(NSString*)message
                    from:(NSString*)senderId
                    to:(NSString*)targetType;
/**
  @brief Triggers when a user joined conference.
  @param user The user joined.
*/
- (void)conferenceClient:(OWTConferenceClient*)client
       didAddParticipant:(OWTConferenceParticipant*)user;
@end
NS_ASSUME_NONNULL_END
#endif  // OWT_CONFERENCE_OBJC_OWTCONFERENCECLIENT_H_
