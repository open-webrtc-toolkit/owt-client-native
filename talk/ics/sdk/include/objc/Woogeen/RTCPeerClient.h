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

#ifndef p2p_PeerClient_h
#define p2p_PeerClient_h

#import <Foundation/Foundation.h>
#import <WebRTC/RTCMacros.h>
#import "Woogeen/RTCConnectionStats.h"
#import "Woogeen/RTCP2PSignalingChannelProtocol.h"
#import "Woogeen/RTCPeerClientObserver.h"
#import "Woogeen/RTCP2PSignalingSenderProtocol.h"
#import "Woogeen/RTCLocalStream.h"
#import "Woogeen/RTCP2PPeerConnectionChannelObserver.h"
#import "Woogeen/RTCPeerClientConfiguration.h"

NS_ASSUME_NONNULL_BEGIN

/// An async client for P2P WebRTC sessions
RTC_EXPORT
@interface RTCPeerClient : NSObject<RTCP2PPeerConnectionChannelObserver,
                                    RTCP2PSignalingSenderProtocol,
                                    RTCP2PSignalingChannelObserver>

/**
 @brief Init an PeerClient instance with speficied signaling channel.
 @param configuration Configuration for creating the PTCPeerClient.
 @param signalingChannel Signaling channel used for exchange signaling messages.
 */
- (instancetype)initWithConfiguration:(RTCPeerClientConfiguration*)configuration
                     signalingChannel:
                         (id<RTCP2PSignalingChannelProtocol>)signalingChannel;

/**
 @brief Add an observer for PeerClient
 @param observer An instance implemented PeerClientObserver protocol.
 */
- (void)addObserver:(id<RTCPeerClientObserver>)observer;

/**
 @brief Remove an observer from PeerClient
 @param observer An instance implemented PeerClientObserver protocol.
 @private
 */
- (void)removeObserver:(id<RTCPeerClientObserver>)observer;

/**
 @brief Connect to the signaling server.
 @param token A token used for connection and authentication
 @param onSuccess Sucess callback will be invoked with current user's ID if
 connect to server successfully.
 @param onFailure Failure callback will be invoked if one of these cases
 happened:
                1. PeerClient is connecting or connected to a server.
                2. Invalid token.
 */
- (void)connect:(NSString*)token
      onSuccess:(nullable void (^)(NSString*))onSuccess
      onFailure:(nullable void (^)(NSError*))onFailure;

/**
 @brief Disconnect from the signaling server.

 It will stop all active WebRTC sessions.
 @param onSuccess Sucess callback will be invoked if disconnect from server
 successfully.
 @param onFailure Failure callback will be invoked if one of these cases
 happened:
                1. PeerClient haven't connected to a signaling server.
 */
- (void)disconnectWithOnSuccess:(nullable void (^)())onSuccess
                      onFailure:(nullable void (^)(NSError*))onFailure;

/**
 @brief Invite a remote user to start a WebRTC session.
 @param targetId Remote user's ID.
 @param onSuccess Success callback will be invoked if send a invitation
 successfully.
 @param onFailure Failure callback will be invoked if one of the following cases
 happened.
                1. PeerClient is disconnected from the server.
                2. Target ID is nil or target user is offline.
 */
- (void)invite:(NSString*)targetId
     onSuccess:(nullable void (^)())onSuccess
     onFailure:(nullable void (^)(NSError*))onFailure;

/**
 @brief Accept a remote user's request to start a WebRTC session.
 @param targetId Remote user's ID.
 @param onSuccess Success callback will be invoked if send an acceptance
 successfully.
 @param onFailure Failure callback will be invoked if one of the following cases
 happened.
                1. PeerClient is disconnected from the server.
                2. Target ID is nil or target user is offline.
                3. Haven't received an invitation from target user.
 */
- (void)accept:(NSString*)targetId
     onSuccess:(nullable void (^)())onSuccess
     onFailure:(nullable void (^)(NSError*))onFailure;

/**
 @brief Deny a remote user's request to start a WebRTC session.
 @param targetId Remote user's ID.
 @param onSuccess Success callback will be invoked if send deny event
 successfully.
 @param onFailure Failure callback will be invoked if one of the following cases
 happened.
                1. PeerClient is disconnected from the server.
                2. Target ID is nil or target user is offline.
                3. Haven't received an invitation from target user.
 */
- (void)deny:(NSString*)targetId
   onSuccess:(nullable void (^)())onSuccess
   onFailure:(nullable void (^)(NSError*))onFailure;

/**
 @brief Send a message to remote client
 @param targetId Remote user's ID.
 @param message The message to be sent.
 @param onSuccess Success callback will be invoked if send deny event
 successfully.
 @param onFailure Failure callback will be invoked if one of the following cases
 happened.
 1. PeerClient is disconnected from the server.
 2. Target ID is nil or target user is offline.
 3. There is no WebRTC session with target user.
 */
- (void)send:(NSString*)targetId
     message:(NSString*)message
   onSuccess:(nullable void (^)())onSuccess
   onFailure:(nullable void (^)(NSError*))onFailure;

/**
 @brief Stop a WebRTC session.
 @param targetId Remote user's ID.
 @param onSuccess Success callback will be invoked if send stop event
 successfully.
 @param onFailure Failure callback will be invoked if one of the following cases
 happened.
                1. PeerClient is disconnected from the server.
                2. Target ID is nil or target user is offline.
                3. There is no WebRTC session with target user.
 */
- (void)stop:(NSString*)targetId
   onSuccess:(nullable void (^)())onSuccess
   onFailure:(nullable void (^)(NSError*))onFailure;

/**
 @brief Publish a stream to the remote client.
 @param stream The stream which will be published.
 @param to Target user's ID.
 @param onSuccess Success callback will be invoked it the stream is published.
 @param onFailure Failure callback will be invoked if one of these cases
 happened:
                  1. PeerClient is disconnected from server.
                  2. Target ID is nil or user is offline.
                  3. Haven't connected to remote client.
 */
- (void)publish:(RTCLocalStream*)stream
             to:(NSString*)targetId
      onSuccess:(nullable void (^)())onSuccess
      onFailure:(nullable void (^)(NSError*))onFailure;

/**
 @brief Unpublish the stream to the remote client.
 @param stream The stream which will be removed.
 @param targetId Target user's ID.
 @param onSuccess Success callback will be invoked it the stream is unpublished.
 @param onFailure Failure callback will be invoked if one of these cases
 happened:
                 1. PeerClient is disconnected from server.
                 2. Target ID is nil or user is offline.
                 3. Haven't connected to remote client.
                 4. The stream haven't been published.
 */
- (void)unpublish:(RTCLocalStream*)stream
               to:(NSString*)targetId
        onSuccess:(nullable void (^)())onSuccess
        onFailure:(nullable void (^)(NSError*))onFailure;

/**
 @brief Get the connection stats between a remote client
 @param targetId Target user's ID.
 @param onSuccess Success callback will be invoked it the stream is unpublished.
 @param onFailure Failure callback will be invoked if one of these cases
 happened:
                 1. PeerClient is disconnected from server.
                 2. Target ID is nil or user is offline.
                 3. Haven't connected to remote client.
 */
- (void)getConnectionStats:(NSString*)targetId
                 onSuccess:(nullable void (^)(RTCConnectionStats*))onSuccess
                 onFailure:(nullable void (^)(NSError*))onFailure;

@end

NS_ASSUME_NONNULL_END

#endif
