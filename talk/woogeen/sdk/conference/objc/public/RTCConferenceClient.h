//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#ifndef WOOGEEN_CONFERENCE_OBJC_RTCCONFERENCECLIENT_H_
#define WOOGEEN_CONFERENCE_OBJC_RTCCONFERENCECLIENT_H_

#import "RTCLocalStream.h"
#import "RTCRemoteStream.h"
#import "RTCConferenceClientObserver.h"

// An asynchronous class for app to communicate with a conference in MCU
@interface RTCConferenceClient : NSObject

/**
  Connect to the specified room to join a conference.
  @param token Includes the room info which is encrypted.
*/
-(void)join:(NSString*)token onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError*))onFailure;
/**
  Publish the stream to the current room.
  @param stream The stream to be published.
*/
-(void)publish:(RTCLocalStream*)stream onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError*))onFailure;
/**
  Un-publish the stream from the current room.
  @param stream The stream to be unpublished.
*/
-(void)unpublish:(RTCLocalStream*)stream onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError*))onFailure;
/**
  Subscribe a stream from the current room.
  @param stream The remote stream to be subscribed.
  @param onSuccess Success callback with a stream that contains media stream.
*/
-(void)subscribe:(RTCRemoteStream*)stream onSuccess:(void (^)(RTCRemoteStream*))onSuccess onFailure:(void (^)(NSError*))onFailure;
/**
  Un-subscribe the stream from the current room.
  @param stream The stream to be unsubscribed.
*/
-(void)unsubscribe:(RTCRemoteStream*)stream onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError*))onFailure;
/**
  Leave current conference.
*/
-(void)leaveWithOnSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError*))onFailure;
/**
  Add an observer for ConferenceClient.
*/
-(void)addObserver:(id<RTCConferenceClientObserver>)observer;
/**
  Remove an observer from the ConferenceClient.
*/
-(void)removeObserver:(id<RTCConferenceClientObserver>)observer;

@end

#endif  // WOOGEEN_CONFERENCE_OBJC_RTCCONFERENCECLIENT_H_
