// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <Foundation/Foundation.h>
#import <WebRTC/RTCMacros.h>
#import <WebRTC/RTCLegacyStatsReport.h>
#import "OWT/OWTLocalStream.h"
#import "OWT/OWTP2PClientConfiguration.h"
#import "OWT/OWTP2PPeerConnectionChannelObserver.h"
#import "OWT/OWTP2PSignalingChannelProtocol.h"
#import "OWT/OWTP2PSignalingSenderProtocol.h"
NS_ASSUME_NONNULL_BEGIN
@protocol OWTP2PClientDelegate;
@class OWTP2PPublication;
/// An async client for P2P WebRTC sessions
RTC_OBJC_EXPORT
@interface OWTP2PClient : NSObject<OWTP2PPeerConnectionChannelObserver,
                                   OWTP2PSignalingSenderProtocol,
                                   OWTP2PSignalingChannelDelegate>
/**
 @brief Initialize a OWTP2PClient instance with a specific signaling channel.
 @param configuration Configuration for creating the OWTP2PClient.
 @param signalingChannel Signaling channel used for exchange signaling messages.
 */
- (instancetype)initWithConfiguration:(OWTP2PClientConfiguration*)configuration
                     signalingChannel:
                         (id<OWTP2PSignalingChannelProtocol>)signalingChannel;
/**
 @brief Connect to the signaling server.
 @param token A token used for connection and authentication
 @param onSuccess Success callback will be invoked with current user's ID if
 connect to server successfully.
 @param onFailure Failure callback will be invoked if one of these cases
 happened:
                1. OWTP2PClient is connecting or connected to a server.
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
                1. OWTP2PClient haven't connected to a signaling server.
 */
- (void)disconnectWithOnSuccess:(nullable void (^)())onSuccess
                      onFailure:(nullable void (^)(NSError*))onFailure;
/**
 @brief Send a message to remote client
 @param message The message to be sent.
 @param targetId Remote user's ID.
 @param onSuccess Success callback will be invoked if send deny event
 successfully.
 @param onFailure Failure callback will be invoked if one of the following cases
 happened.
 1. OWTP2PClient is disconnected from the server.
 2. Target ID is nil or target user is offline.
 3. There is no WebRTC session with target user.
 */
- (void)send:(NSString*)message
           to:(NSString*)targetId
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
                  1. OWTP2PClient is disconnected from server.
                  2. Target ID is nil or user is offline.
                  3. Haven't connected to remote client.
 */
- (void)publish:(OWTLocalStream*)stream
             to:(NSString*)targetId
      onSuccess:(nullable void (^)(OWTP2PPublication*))onSuccess
      onFailure:(nullable void (^)(NSError*))onFailure;
/**
 @brief Get the connection statistowt with target client.
 @param targetId Remote user's ID.
 @param onSuccess Success callback will be invoked if get statistoms
 information successes.
 @param onFailure Failure callback will be invoked if one of the following
 cases happened.
 1. Target ID is invalid.
 2. There is no WebRTC session with target user.
 */
- (void)statsFor:(NSString*)targetId
       onSuccess:(void (^)(NSArray<RTCLegacyStatsReport*>*))onSuccess
       onFailure:(nullable void (^)(NSError*))onFailure;
@property(nonatomic, weak) id<OWTP2PClientDelegate> delegate;
@property(nonatomic, strong) NSMutableArray<NSString*>* allowedRemoteIds;
@end
/// Delegate for OWTConferenceClient.
RTC_OBJC_EXPORT
@protocol OWTP2PClientDelegate<NSObject>
@optional
/**
  @brief Triggered when client is disconnected from signaling server.
*/
- (void)p2pClientDidDisconnect:(OWTP2PClient*)client;
/**
  @brief Triggered when a stream is added.
  @param stream The stream which is added.
*/
- (void)p2pClient:(OWTP2PClient*)client didAddStream:(OWTRemoteStream*)stream;
/**
  @brief Triggered when a message is received.
  @param senderId Sender's ID.
  @param message Message received.
*/
- (void)p2pClient:(OWTP2PClient*)client
    didReceiveMessage:(NSString*)message
                 from:(NSString*)senderId;
@end
NS_ASSUME_NONNULL_END
