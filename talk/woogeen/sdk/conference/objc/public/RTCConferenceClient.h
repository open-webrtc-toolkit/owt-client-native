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
  @brief Leave current conference.
*/
-(void)leaveWithOnSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError*))onFailure;
/**
  @brief Add an observer for ConferenceClient.
*/
-(void)addObserver:(id<RTCConferenceClientObserver>)observer;
/**
  @brief Remove an observer from the ConferenceClient.
*/
-(void)removeObserver:(id<RTCConferenceClientObserver>)observer;

@end

#endif  // WOOGEEN_CONFERENCE_OBJC_RTCCONFERENCECLIENT_H_
