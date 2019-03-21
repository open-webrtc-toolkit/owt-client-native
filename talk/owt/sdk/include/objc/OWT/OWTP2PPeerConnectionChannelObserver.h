// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <Foundation/Foundation.h>
#import "OWT/OWTRemoteStream.h"
/// @cond
/// OWTP2PPeerConnectionChannelObserver is an ObjectiveC wrapper for
/// P2PPeerConnectionChannelObserver.
@protocol OWTP2PPeerConnectionChannelObserver<NSObject>
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
/// @endcond
