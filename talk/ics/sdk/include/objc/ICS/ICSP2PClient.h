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

#import <Foundation/Foundation.h>
#import <WebRTC/RTCMacros.h>
#import "ICS/ICSConnectionStats.h"
#import "ICS/ICSLocalStream.h"
#import "ICS/ICSP2PClientConfiguration.h"
#import "ICS/ICSP2PPeerConnectionChannelObserver.h"
#import "ICS/ICSP2PSignalingChannelProtocol.h"
#import "ICS/ICSP2PSignalingSenderProtocol.h"

NS_ASSUME_NONNULL_BEGIN

@protocol ICSP2PClientDelegate;
@class ICSP2PPublication;

/// An async client for P2P WebRTC sessions
RTC_EXPORT
@interface ICSP2PClient : NSObject<ICSP2PPeerConnectionChannelObserver,
                                   ICSP2PSignalingSenderProtocol,
                                   ICSP2PSignalingChannelDelegate>

/**
 @brief Initialize a ICSP2PClient instance with a specific signaling channel.
 @param configuration Configuration for creating the ICSP2PClient.
 @param signalingChannel Signaling channel used for exchange signaling messages.
 */
- (instancetype)initWithConfiguration:(ICSP2PClientConfiguration*)configuration
                     signalingChannel:
                         (id<ICSP2PSignalingChannelProtocol>)signalingChannel;

/**
 @brief Connect to the signaling server.
 @param token A token used for connection and authentication
 @param onSuccess Success callback will be invoked with current user's ID if
 connect to server successfully.
 @param onFailure Failure callback will be invoked if one of these cases
 happened:
                1. ICSP2PClient is connecting or connected to a server.
                2. Invalid token.
 */
- (void)connect:(NSString*)token
      onSuccess:(nullable void (^)(NSString*))onSuccess
      onFailure:(nullable void (^)(NSError*))onFailure;

/**
 @brief Disconnect from the signaling server.

 It will stop all active WebRTC sessions.
 @param onSuccess Success callback will be invoked if disconnect from server
 successfully.
 @param onFailure Failure callback will be invoked if one of these cases
 happened:
                1. ICSP2PClient haven't connected to a signaling server.
 */
- (void)disconnectWithOnSuccess:(nullable void (^)())onSuccess
                      onFailure:(nullable void (^)(NSError*))onFailure;

/**
 @brief Send a message to remote client
 @param targetId Remote user's ID.
 @param message The message to be sent.
 @param onSuccess Success callback will be invoked if send deny event
 successfully.
 @param onFailure Failure callback will be invoked if one of the following cases
 happened.
 1. ICSP2PClient is disconnected from the server.
 2. Target ID is nil or target user is offline.
 3. There is no WebRTC session with target user.
 */
- (void)send:(NSString*)targetId
      message:(NSString*)message
    onSuccess:(nullable void (^)())onSuccess
    onFailure:(nullable void (^)(NSError*))onFailure;

/**
 @brief Stop a WebRTC session.
 @details Clean all resources associated with given remote endpoint. It may include RTCPeerConnection, RTCRtpTransceiver and RTCDataChannel. It still possible to publish a stream, or send a message to given remote endpoint after stop.
 @param targetId Remote user's ID.
 */
- (void)stop:(NSString*)targetId;

/**
 @brief Publish a stream to the remote client.
 @param stream The stream which will be published.
 @param to Target user's ID.
 @param onSuccess Success callback will be invoked it the stream is published.
 @param onFailure Failure callback will be invoked if one of these cases
 happened:
                  1. ICSP2PClient is disconnected from server.
                  2. Target ID is nil or user is offline.
                  3. Haven't connected to remote client.
 */
- (void)publish:(ICSLocalStream*)stream
             to:(NSString*)targetId
      onSuccess:(nullable void (^)(ICSP2PPublication*))onSuccess
      onFailure:(nullable void (^)(NSError*))onFailure;

@property(nonatomic, weak) id<ICSP2PClientDelegate> delegate;
@property(nonatomic, strong) NSMutableArray<NSString*>* allowedRemoteIds;

@end

/// Delegate for ICSConferenceClient.
RTC_EXPORT
@protocol ICSP2PClientDelegate<NSObject>

@optional
/**
  @brief Triggered when client is disconnected from signaling server.
*/
- (void)p2pClientDidDisconnect:(ICSP2PClient*)client;

/**
  @brief Triggered when a stream is added.
  @param stream The stream which is added.
*/
- (void)p2pClient:(ICSP2PClient*)client didAddStream:(ICSRemoteStream*)stream;

/**
  @brief Triggered when a message is received.
  @param senderId Sender's ID.
  @param message Message received.
*/
- (void)p2pClient:(ICSP2PClient*)client
    didReceiveMessage:(NSString*)message
                 from:(NSString*)senderId;

@end

NS_ASSUME_NONNULL_END
