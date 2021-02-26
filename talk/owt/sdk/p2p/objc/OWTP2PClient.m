// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "RTCLogging.h"
#import "talk/owt/sdk/include/objc/OWT/OWTP2PClient.h"
#import "talk/owt/sdk/include/objc/OWT/OWTP2PErrors.h"
#import "talk/owt/sdk/p2p/objc/OWTP2PPeerConnectionChannel.h"
@interface OWTP2PClient ()
- (OWTP2PPeerConnectionChannel*)getPeerConnectionChannel:(NSString*)targetId;
- (void)removePeerConnectionChannel:(NSString*)targetId;
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
- (void)unpublish:(OWTLocalStream*)stream
               to:(NSString*)targetId
        onSuccess:(nullable void (^)(void))onSuccess
        onFailure:(nullable void (^)(NSError*))onFailure;
@end
typedef enum { kDisconnected, kConnecting, kConnected } SignalingChannelState;
@implementation OWTP2PClient {
  id<OWTP2PSignalingChannelProtocol> _signalingChannel;
  SignalingChannelState _peerClientState;
  NSMutableDictionary* _peerConnectionChannels;
  NSString* _localId;
  OWTP2PClientConfiguration* _configuration;
}
- (instancetype)initWithConfiguration:(OWTP2PClientConfiguration*)configuration
                signalingChannel:
                    (id<OWTP2PSignalingChannelProtocol>)signalingChannel {
  self = [super init];
  _signalingChannel = signalingChannel;
  _signalingChannel.delegate = self;
  _peerConnectionChannels = [[NSMutableDictionary alloc] init];
  _configuration = configuration;
  _peerClientState = kDisconnected;
  return self;
}
- (void)connect:(NSString*)token
      onSuccess:(void (^)(NSString*))onSuccess
      onFailure:(void (^)(NSError*))onFailure {
  _peerClientState = kConnecting;
  [_signalingChannel connect:token
      onSuccess:^(NSString* myId) {
        self->_peerClientState = kConnected;
        self->_localId = myId;
        if (onSuccess) {
          onSuccess(myId);
        }
      }
      onFailure:^(NSError* err) {
        self->_peerClientState = kDisconnected;
        if (onFailure) {
          onFailure(err);
        }
      }];
}
- (void)disconnectWithOnSuccess:(void (^)(void))onSuccess
                      onFailure:(void (^)(NSError*))onFailure {
  for (id key in _peerConnectionChannels) {
    OWTP2PPeerConnectionChannel* channel =
        [_peerConnectionChannels objectForKey:key];
    [channel stopWithOnSuccess:nil onFailure:nil];
    [_peerConnectionChannels removeObjectForKey:key];
  }
  [_signalingChannel disconnectWithOnSuccess:onSuccess onFailure:onFailure];
}
- (BOOL)checkSignalingChannelOnline:(void (^)(NSError*))failure {
  if (_peerClientState != kConnected) {
    if (failure) {
      NSError* err = [[NSError alloc]
          initWithDomain:OWTErrorDomain
                    code:OWTP2PErrorClientInvalidState
                userInfo:[[NSDictionary alloc]
                             initWithObjectsAndKeys:@"PeerClient hasn't "
                                                    @"connected to a signaling"
                                                    @" server.",
                                                    NSLocalizedDescriptionKey,
                                                    nil]];
      failure(err);
    }
    return NO;
  }
  return YES;
}
- (void)publish:(OWTLocalStream*)stream
             to:(NSString*)targetId
      onSuccess:(void (^)(OWTP2PPublication*))onSuccess
      onFailure:(void (^)(NSError*))onFailure {
  if (![self checkSignalingChannelOnline:onFailure])
    return;
  OWTP2PPeerConnectionChannel* channel =
      [self getPeerConnectionChannel:targetId];
  [channel publish:stream onSuccess:onSuccess onFailure:onFailure];
}
- (void)unpublish:(OWTLocalStream*)stream
               to:(NSString*)targetId
        onSuccess:(void (^)(void))onSuccess
        onFailure:(void (^)(NSError*))onFailure {
  if (![self checkSignalingChannelOnline:onFailure])
    return;
  OWTP2PPeerConnectionChannel* channel =
      [self getPeerConnectionChannel:targetId];
  [channel unpublish:stream onSuccess:onSuccess onFailure:onFailure];
}
- (OWTP2PPeerConnectionChannel*)getPeerConnectionChannel:(NSString*)targetId {
  OWTP2PPeerConnectionChannel* channel =
      [_peerConnectionChannels objectForKey:targetId];
  if (channel == nil) {
    channel = [[OWTP2PPeerConnectionChannel alloc]
        initWithConfiguration:_configuration
                      localId:_localId
                     remoteId:targetId
              signalingSender:self];
    [channel addObserver:self];
    [_peerConnectionChannels setObject:channel forKey:targetId];
  }
  return channel;
}

- (void)removePeerConnectionChannel:(NSString*)targetId {
  OWTP2PPeerConnectionChannel* channel =
      [_peerConnectionChannels objectForKey:targetId];
  if (channel) {
    [channel removeObserver:self];
    [_peerConnectionChannels removeObjectForKey:targetId];
  }
}

- (void)sendSignalingMessage:(NSString*)data
                          to:(NSString*)targetId
                   onSuccess:(void (^)(void))onSuccess
                   onFailure:(void (^)(NSError*))onFailure {
  [_signalingChannel sendMessage:data
                              to:targetId
                       onSuccess:onSuccess
                       onFailure:onFailure];
}
- (void)stop:(NSString*)targetId {
  if (![self checkSignalingChannelOnline:nil])
    return;
  OWTP2PPeerConnectionChannel* channel =
      [self getPeerConnectionChannel:targetId];
  [channel stopWithOnSuccess:nil onFailure:nil];
  [_peerConnectionChannels removeObjectForKey:targetId];
}
- (void)send:(NSString*)message
           to:(NSString*)targetId
    onSuccess:(nullable void (^)(void))onSuccess
    onFailure:(nullable void (^)(NSError*))onFailure {
  if (![self checkSignalingChannelOnline:onFailure])
    return;
  OWTP2PPeerConnectionChannel* channel =
      [self getPeerConnectionChannel:targetId];
  [channel send:message withOnSuccess:onSuccess onFailure:onFailure];
}
- (void)statsFor:(NSString*)targetId
       onSuccess:(void (^)(NSArray<RTCLegacyStatsReport*>*))onSuccess
       onFailure:(nullable void (^)(NSError*))onFailure {
  OWTP2PPeerConnectionChannel* channel =
      [self getPeerConnectionChannel:targetId];
  [channel statsWithOnSuccess:onSuccess onFailure:onFailure];
}
- (void)channel:(id<OWTP2PSignalingChannelProtocol>)channel
    didReceiveMessage:(NSString*)message
                 from:(NSString*)senderId {
  if (![self.allowedRemoteIds containsObject:senderId]) {
    RTCLogInfo(@"Receive signaling message from disallowed user: %@.",
               senderId);
    return;
  }
  if ([message isEqualToString:@"{\"type\":\"chat-closed\"}"] &&
      [_peerConnectionChannels objectForKey:senderId] == nil) {
    return;
  }
  OWTP2PPeerConnectionChannel* pcChannel =
      [self getPeerConnectionChannel:senderId];
  [pcChannel onIncomingSignalingMessage:message];
}
- (void)onInvitedFrom:(NSString*)remoteUserId {
  RTCLogInfo(@"On invited from %@", remoteUserId);
}
- (void)onStreamAdded:(OWTRemoteStream*)stream {
  RTCLogInfo(@"PeerClient received stream add.");
  if ([_delegate respondsToSelector:@selector(p2pClient:didAddStream:)]) {
    [_delegate p2pClient:self didAddStream:stream];
  }
}
- (void)onStreamRemoved:(OWTRemoteStream*)stream {
  RTCLogInfo(@"PeerClient received stream removed.");
}
- (void)onAcceptedFrom:(NSString*)remoteUserId {
  RTCLogInfo(@"PeerClient received accepted.");
}
- (void)onDeniedFrom:(NSString*)remoteUserId {
  RTCLogInfo(@"PeerClient received Denied.");
  [_peerConnectionChannels removeObjectForKey: remoteUserId];
}
- (void)onDataReceivedFrom:(NSString*)remoteUserId withData:(NSString*)data {
  RTCLogInfo(@"Received data from data channel.");
  if ([_delegate
          respondsToSelector:@selector(p2pClient:didReceiveMessage:from:)]) {
    [_delegate p2pClient:self
        didReceiveMessage:data
                     from:remoteUserId];
  }
}
- (void)channelDidDisconnect:(id<OWTP2PSignalingChannelProtocol>)channel {
  RTCLogInfo(@"PeerClient received disconnect.");
  _peerClientState = kDisconnected;
  if ([_delegate respondsToSelector:@selector(p2pClientDidDisconnect:)]) {
    [_delegate p2pClientDidDisconnect:self];
  }
}

- (void)onStoppedFrom:(NSString*)remoteUserId {
  RTCLogInfo(@"PeerClient received chat stopped.");
  [self removePeerConnectionChannel:remoteUserId];
}

- (void)onStartedFrom:(NSString*)remoteUserId {
  RTCLogInfo(@"PeerClient received chat started.");
}
@end
