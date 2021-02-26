// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "RTCLegacyStatsReport.h"
#import "RTCLogging.h"
#import "talk/owt/sdk/include/objc/OWT/OWTP2PPublication.h"
#import "talk/owt/sdk/include/objc/OWT/OWTP2PPeerConnectionChannelObserver.h"
@interface OWTP2PPublication () <OWTP2PPeerConnectionChannelObserver>
@property(nonatomic, readonly) void (^stopMethod)(void);
@property(nonatomic, readonly) void (^statsMethod)
    (void (^)(NSArray<RTCLegacyStatsReport*>*), void (^)(NSError*));
- (instancetype)initWithStop:(void (^)(void))stopMethod
                       stats:
                           (void (^)(void (^)(NSArray<RTCLegacyStatsReport*>*),
                                     void (^)(NSError*)))statsMethod;
/**
  @brief This function will be invoked when received a invitation.
  @param remoteUserId Remote user’s ID
*/
- (void)onInvitedFrom:(NSString*)remoteUserId;
/**
  @brief This function will be invoked when a remote user accepted current
  user's invitation.
  @param remoteUserId Remote user’s ID
*/
- (void)onAcceptedFrom:(NSString*)remoteUserId;
/**
  @brief This function will be invoked when a remote user denied current user's
  invitation.
  @param remoteUserId Remote user’s ID
*/
- (void)onDeniedFrom:(NSString*)remoteUserId;
/**
  @brief This function will be invoked when a chat is started. (This event
  haven't been implemented yet)
  @param remoteUserId Remote user’s ID
*/
- (void)onStoppedFrom:(NSString*)remoteUserId;
/**
  @brief This function will be invoked when a chat is stopped. (This event
  haven't been implemented yet)
  @param remoteUserId Remote user’s ID
*/
- (void)onStartedFrom:(NSString*)remoteUserId;
/**
  @brief This function will be invoked when received data from a remote user.
  (This event haven't been implemented yet)
  @param remoteUserId Remote user’s ID
  @param data Data received.
*/
- (void)onDataReceivedFrom:(NSString*)remoteUserId withData:(NSString*)data;
/**
  @brief This function will be invoked when a remote stream is available.
  @param remoteUserId Remote user’s ID
*/
- (void)onStreamAdded:(OWTRemoteStream*)stream;
/**
  @brief This function will be invoked when a remote stream is removed.
  @param remoteUserId Remote user’s ID
*/
- (void)onStreamRemoved:(OWTRemoteStream*)stream;
@end
