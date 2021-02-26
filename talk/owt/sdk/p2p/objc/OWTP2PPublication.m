// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "RTCLogging.h"
#import "talk/owt/sdk/p2p/objc/OWTP2PPublication+Private.h"
@implementation OWTP2PPublication
- (instancetype)initWithStop:(void (^)(void))stopMethod
                       stats:
                           (void (^)(void (^)(NSArray<RTCLegacyStatsReport*>*),
                                     void (^)(NSError*)))statsMethod {
  if (self = [super init]) {
    _stopMethod = stopMethod;
    _statsMethod = statsMethod;
  }
  return self;
}
- (void)stop {
  _stopMethod();
  if ([_delegate respondsToSelector:@selector(publicationDidEnd:)]) {
    [_delegate publicationDidEnd:self];
  }
}
- (void)stats:(void (^)(NSArray<RTCLegacyStatsReport*>* stats))onSuccess
    onFailure:(nullable void (^)(NSError*))onFailure {
  _statsMethod(onSuccess, onFailure);
}
/**
  @brief This function will be invoked when received a invitation.
  @param remoteUserId Remote user’s ID
*/
- (void)onInvitedFrom:(NSString*)remoteUserId {
}
/**
  @brief This function will be invoked when a remote user accepted current
  user's invitation.
  @param remoteUserId Remote user’s ID
*/
- (void)onAcceptedFrom:(NSString*)remoteUserId {
}
/**
  @brief This function will be invoked when a remote user denied current user's
  invitation.
  @param remoteUserId Remote user’s ID
*/
- (void)onDeniedFrom:(NSString*)remoteUserId {
}
/**
  @brief This function will be invoked when a chat is started. (This event
  haven't been implemented yet)
  @param remoteUserId Remote user’s ID
*/
- (void)onStoppedFrom:(NSString*)remoteUserId {
  if ([_delegate respondsToSelector:@selector(publicationDidEnd:)]) {
    [_delegate publicationDidEnd:self];
  }
}
/**
  @brief This function will be invoked when a chat is stopped. (This event
  haven't been implemented yet)
  @param remoteUserId Remote user’s ID
*/
- (void)onStartedFrom:(NSString*)remoteUserId {
}
/**
  @brief This function will be invoked when received data from a remote user.
  (This event haven't been implemented yet)
  @param remoteUserId Remote user’s ID
  @param data Data received.
*/
- (void)onDataReceivedFrom:(NSString*)remoteUserId withData:(NSString*)data {
}
/**
  @brief This function will be invoked when a remote stream is available.
  @param remoteUserId Remote user’s ID
*/
- (void)onStreamAdded:(OWTRemoteStream*)stream {
}
/**
  @brief This function will be invoked when a remote stream is removed.
  @param remoteUserId Remote user’s ID
*/
- (void)onStreamRemoved:(OWTRemoteStream*)stream {
}
@end
