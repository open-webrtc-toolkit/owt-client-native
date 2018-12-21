// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef p2p_SignalingChannelProtocol_h
#define p2p_SignalingChannelProtocol_h
#import "OMS/OMSErrors.h"
@protocol OMSP2PSignalingChannelDelegate;
/**
 @brief Protocol for signaling channel.
 Developers may utilize their own signaling server by implementing this protocol.
 */
RTC_EXPORT
@protocol OMSP2PSignalingChannelProtocol<NSObject>
@optional
@property(nonatomic, weak) id<OMSP2PSignalingChannelDelegate> delegate;
/**
 @brief Connect to the signaling server
 @param token A token used for connecting signaling server
 */
- (void)connect:(NSString*)token
      onSuccess:(void (^)(NSString*))onSuccess
      onFailure:(void (^)(NSError*))onFailure;
/**
 @brief Send a message to a target client
 @param message Message needs to be send to signaling server
 @param targetId Target user's ID.
 */
- (void)sendMessage:(NSString*)message
                 to:(NSString*)targetId
          onSuccess:(void (^)())onSuccess
          onFailure:(void (^)(NSError*))onFailure;
/**
 @brief Disconnect from signaling server.
 */
- (void)disconnectWithOnSuccess:(void (^)())onSuccess
                      onFailure:(void (^)(NSError*))onFailure;
@end
/**
 @brief Signaling channel will notify observer when event triggers.
 */
@protocol OMSP2PSignalingChannelDelegate<NSObject>
/**
 @brief This function will be triggered when new message arrives.
 @param message Message received from signaling server.
 @param senderId Sender's ID.
 */
- (void)channel:(id<OMSP2PSignalingChannelProtocol>)channel
    didReceiveMessage:(NSString*)message
                 from:(NSString*)senderId;
/**
 @brief This function will be triggered when disconnected from signaling server.
 */
- (void)channelDidDisconnect:(id<OMSP2PSignalingChannelProtocol>)channel;
@end
#endif
