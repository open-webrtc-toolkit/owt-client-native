//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#ifndef WOOGEEN_CONFERENCE_OBJC_RTCCONFERENCECLIENTOBSERVER_H_
#define WOOGEEN_CONFERENCE_OBJC_RTCCONFERENCECLIENTOBSERVER_H_

#import "RTCLocalStream.h"
#import "RTCRemoteStream.h"
#import "RTCConferenceUser.h"

/// Observer for RTCConferenceClient.
@protocol RTCConferenceClientObserver <NSObject>

/**
  @brief Triggers when client is disconnected from conference server.
*/
-(void)onServerDisconnected;
/**
  @brief Triggers when a stream is added.
  @param stream The stream which is added.
*/
-(void)onStreamAdded:(RTCRemoteStream*)stream;
/**
  @brief Triggers when a stream is removed.
  @param stream The stream which is removed.
*/
-(void)onStreamRemoved:(RTCRemoteStream*)stream;
/**
  @brief Triggers when a message is received.
  @param senderId Sender's ID.
  @param message Message received.
*/
-(void)onMessageReceivedFrom:(NSString*) senderId message:(NSString*)message;
/**
  @brief Triggers when a user joined conference.
  @param user The user joined.
*/
-(void)onUserJoined:(RTCConferenceUser*)user;
/**
  @brief Triggers when a user left conference.
  @param user The user left.
*/
-(void)onUserLeft:(RTCConferenceUser*)user;

@end

#endif  // WOOGEEN_CONFERENCE_OBJC_RTCCONFERENCECLIENTOBSERVER_H_
