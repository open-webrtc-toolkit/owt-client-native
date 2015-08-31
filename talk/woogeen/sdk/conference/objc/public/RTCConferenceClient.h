//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#ifndef WOOGEEN_CONFERENCE_OBJC_RTCCONFERENCECLIENT_H_
#define WOOGEEN_CONFERENCE_OBJC_RTCCONFERENCECLIENT_H_

#import "RTCLocalStream.h"
#import "RTCRemoteStream.h"
#import "RTCConferenceClientObserver.h"
#import "RTCConferenceClientConfiguration.h"

/// An asynchronous class for app to communicate with a conference in MCU
@interface RTCConferenceClient : NSObject

/**
  @brief Initialize a RTCConferenceClient with configuration
  @param config Configuration for creating the RTCConferenceClient.
*/
-(instancetype)initWithConfiguration:(RTCConferenceClientConfiguration*)config;
/**
  @brief Connect to the specified room to join a conference.
  @param token Includes the room info which is encrypted.
*/
-(void)joinWithOnSuccess:(NSString*)token onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError*))onFailure;
/**
  @brief Publish the stream to the current room.
  @param stream The stream to be published.
*/
-(void)publish:(RTCLocalStream*)stream onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError*))onFailure;
/**
  @brief Un-publish the stream from the current room.
  @param stream The stream to be unpublished.
*/
-(void)unpublish:(RTCLocalStream*)stream onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError*))onFailure;
/**
  @brief Subscribe a stream from the current room.
  @param stream The remote stream to be subscribed.
  @param onSuccess Success callback with a stream that contains media stream.
*/
-(void)subscribe:(RTCRemoteStream*)stream onSuccess:(void (^)(RTCRemoteStream*))onSuccess onFailure:(void (^)(NSError*))onFailure;
/**
  @brief Un-subscribe the stream from the current room.
  @param stream The stream to be unsubscribed.
*/
-(void)unsubscribe:(RTCRemoteStream*)stream onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError*))onFailure;
/**
  @brief Send messsage to all participants in the conference.
  @param message The message to be sent.
*/
-(void)send:(NSString*)message onSuccess:(void (^)())onSuccess onFailure:(void(^)(NSError*))onFailure;
/**
  @brief Continue to transmit specified stream's audio data.
  @detail If |stream| is a remote stream, MCU will continue to send audio data to client. If |stream| is a local stream, client will cont inue to send audio data to MCU. This method is expected to be called after |disableAudio|.
  @param This stream's audio will be played.
*/
-(void)playAudio:(RTCStream*)stream onSuccess:(void (^)())onSuccess onFailure:(void(^)(NSError*))onFailure;
/**
  @brief Stop transmitting specified stream's audio data.
  @detail If |stream| is a remote stream, MCU will stop sending audio data to client. If |stream| is a local stream, client will stop sending audio data to MCU.
  @param This stream's audio will be paused.
*/
-(void)pauseAudio:(RTCStream*)stream onSuccess:(void (^)())onSuccess onFailure:(void(^)(NSError*))onFailure;
/**
  @brief Continue to transmit specified stream's video data.
  @detail If |stream| is a remote stream, MCU will continue to send video data to client. If |stream| is a local stream, client will cont inue to send video data to MCU. This method is expected to be called after |disableVideo|.
  @param This stream's video will be played.
*/
-(void)playVideo:(RTCStream*)stream onSuccess:(void (^)())onSuccess onFailure:(void(^)(NSError*))onFailure;
/**
  @brief Stop transmitting specified stream's video data.
  @detail If |stream| is a remote stream, MCU will stop sending video data to client. If |stream| is a local stream, client will stop sending video data to MCU.
  @param This stream's video will be played.
*/
-(void)pauseVideo:(RTCStream*)stream onSuccess:(void (^)())onSuccess onFailure:(void(^)(NSError*))onFailure;
/**
  @brief Leave current conference.
*/
-(void)leaveWithOnSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError*))onFailure;
/**
  @brief Add an observer for ConferenceClient.
*/
-(void)addObserver:(id<RTCConferenceClientObserver>)observer;
/// @cond
/**
  @brief Remove an observer from the ConferenceClient.
*/
-(void)removeObserver:(id<RTCConferenceClientObserver>)observer;
/// @endcond

@end

#endif  // WOOGEEN_CONFERENCE_OBJC_RTCCONFERENCECLIENT_H_
