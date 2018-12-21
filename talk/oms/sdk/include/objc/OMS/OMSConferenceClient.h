// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
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
