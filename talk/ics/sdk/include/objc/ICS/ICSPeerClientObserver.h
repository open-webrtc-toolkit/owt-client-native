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

#ifndef p2p_PeerClientObserver_h
#define p2p_PeerClientObserver_h

#import <Foundation/Foundation.h>
#import "ICS/ICSLocalStream.h"
#import "ICS/ICSRemoteStream.h"

/// Observer for RTCPeerClient
RTC_EXPORT
@protocol RTCPeerClientObserver<NSObject>

/**
 @brief This function will be invoked when client is disconnected from signaling
 server.
 */
- (void)onServerDisconnected;
/**
 @brief This function will be invoked when received a invitation.
 @param remoteUserId Remote user’s ID
 */
- (void)onInvited:(NSString*)remoteUserId;
/**
 @brief This function will be invoked when a remote user denied current user's
 invitation.
 @param remoteUserId Remote user’s ID
 */
- (void)onDenied:(NSString*)remoteUserId;
/**
 @brief This function will be invoked when a remote user accepted current user's
 invitation.
 @param remoteUserId Remote user’s ID
 */
- (void)onAccepted:(NSString*)remoteUserId;
/**
 @brief This function will be invoked when a chat is stopped. (This event
 haven't been implemented yet)
 @param remoteUserId Remote user’s ID
 */
- (void)onChatStopped:(NSString*)remoteUserId;
/**
 @brief This function will be invoked when a chat is started. (This event
 haven't been implemented yet)
 @param remoteUserId Remote user’s ID
 */
- (void)onChatStarted:(NSString*)remoteUserId;
/**
 @brief This function will be invoked when received data from a remote user.
 (This event haven't been implemented yet)
 @param remoteUserId Remote user’s ID
 @param message Message received
 */
- (void)onDataReceived:(NSString*)remoteUserId message:(NSString*)message;
/**
 @brief This function will be invoked when a remote stream is available.
 @param stream The remote stream added.
 */
- (void)onStreamAdded:(ICSRemoteStream*)stream;
/**
 @brief This function will be invoked when a remote stream is removed.
 @param stream The remote stream removed.
 */
- (void)onStreamRemoved:(ICSRemoteStream*)stream;

@end

#endif
