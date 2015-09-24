/*
 * Copyright © 2015 Intel Corporation. All Rights Reserved.
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
