//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#ifndef WOOGEEN_CONFERENCE_OBJC_RTCCONFERENCECLIENTOBSERVER_H_
#define WOOGEEN_CONFERENCE_OBJC_RTCCONFERENCECLIENTOBSERVER_H_

#import "RTCLocalStream.h"
#import "RTCRemoteStream.h"

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
// TODO(jianjun): Enable user events
//-(void)onUserJoined:(User*)user;
//-(void)onUserLeft:(User*)user;

@end

#endif  // WOOGEEN_CONFERENCE_OBJC_RTCCONFERENCECLIENTOBSERVER_H_
