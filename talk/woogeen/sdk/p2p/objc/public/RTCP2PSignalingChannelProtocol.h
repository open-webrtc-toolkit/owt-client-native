//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#ifndef p2p_SignalingChannelProtocol_h
#define p2p_SignalingChannelProtocol_h

#import "RTCErrors.h"

/**
 @brief Signaling channel will notify observer when event triggers.
 */
@protocol RTCP2PSignalingChannelObserver <NSObject>

/**
 @brief This function will be triggered when new message arrives.
 @param message Message received from signaling server.
 @param senderId Sender's ID.
 */
-(void)onMessage:(NSString *)message from:(NSString *)senderId;
/**
 @brief This function will be triggered when disconnected from signaling server.
 */
-(void)onDisconnected;

@end

/**
 @brief Protocol for signaling channel.

 Developers may utilize their own signaling server by implmenting this protocol.
 */
@protocol RTCP2PSignalingChannelProtocol <NSObject>

/**
 @brief Add an observer for RTCP2PSignalingChannel
 @param observer An observer instance.
 */
-(void)addObserver:(id<RTCP2PSignalingChannelObserver>)observer;

/**
 @brief Remove an observer for RTCP2PSignalingChannel
 @param observer An observer instance.
 */
-(void)removeObserver:(id<RTCP2PSignalingChannelObserver>)observer;

/**
 @brief Connect to the signaling server
 @param token A token used for connecting signaling server
 */
-(void)connect:(NSString *)token onSuccess:(void (^)(NSString *))onSuccess onFailure:(void (^)(NSError*))onFailure;

/**
 @brief Send a message to a target client
 @param message Message needs to be send to signaling server
 @param targetId Target user's ID.
 */
-(void)sendMessage:(NSString *)message to:(NSString *)targetId onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError *))onFailure;

/**
 @brief Disconnect from signaling server.
 */
-(void)disconnectWithOnSuccess:(void(^)())onSuccess onFailure:(void(^)(NSError *))onFailure;

@end


#endif