//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//
#import <WebRTC/RTCLogging.h>
#import "talk/oms/sdk/include/objc/OMS/OMSP2PClient.h"
#import "talk/oms/sdk/include/objc/OMS/OMSP2PErrors.h"
#import "talk/oms/sdk/p2p/objc/OMSP2PPeerConnectionChannel.h"
@interface OMSP2PClient ()
- (OMSP2PPeerConnectionChannel*)getPeerConnectionChannel:(NSString*)targetId;
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
- (void)unpublish:(OMSLocalStream*)stream
               to:(NSString*)targetId
        onSuccess:(nullable void (^)())onSuccess
        onFailure:(nullable void (^)(NSError*))onFailure;
@end
typedef enum { kDisconnected, kConnecting, kConnected } SignalingChannelState;
@implementation OMSP2PClient {
  id<OMSP2PSignalingChannelProtocol> _signalingChannel;
  SignalingChannelState _peerClientState;
  NSMutableDictionary* _peerConnectionChannels;
  NSString* _localId;
  OMSP2PClientConfiguration* _configuration;
}
- (instancetype)initWithConfiguration:(OMSP2PClientConfiguration*)configuration
                signalingChannel:
                    (id<OMSP2PSignalingChannelProtocol>)signalingChannel {
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
        _peerClientState = kConnected;
        _localId = myId;
        if (onSuccess) {
          onSuccess(myId);
        }
      }
      onFailure:^(NSError* err) {
        _peerClientState = kDisconnected;
        if (onFailure) {
          onFailure(err);
        }
      }];
}
- (void)disconnectWithOnSuccess:(void (^)())onSuccess
                      onFailure:(void (^)(NSError*))onFailure {
  for (id key in _peerConnectionChannels) {
    OMSP2PPeerConnectionChannel* channel =
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
          initWithDomain:OMSErrorDomain
                    code:OMSP2PErrorClientInvalidState
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
- (void)publish:(OMSLocalStream*)stream
             to:(NSString*)targetId
      onSuccess:(void (^)(OMSP2PPublication*))onSuccess
      onFailure:(void (^)(NSError*))onFailure {
  if (![self checkSignalingChannelOnline:onFailure])
    return;
  OMSP2PPeerConnectionChannel* channel =
      [self getPeerConnectionChannel:targetId];
  [channel publish:stream onSuccess:onSuccess onFailure:onFailure];
}
- (void)unpublish:(OMSLocalStream*)stream
               to:(NSString*)targetId
        onSuccess:(void (^)())onSuccess
        onFailure:(void (^)(NSError*))onFailure {
  if (![self checkSignalingChannelOnline:onFailure])
    return;
  OMSP2PPeerConnectionChannel* channel =
      [self getPeerConnectionChannel:targetId];
  [channel unpublish:stream onSuccess:onSuccess onFailure:onFailure];
}
- (OMSP2PPeerConnectionChannel*)getPeerConnectionChannel:(NSString*)targetId {
  OMSP2PPeerConnectionChannel* channel =
      [_peerConnectionChannels objectForKey:targetId];
  if (channel == nil) {
    channel = [[OMSP2PPeerConnectionChannel alloc]
        initWithConfiguration:_configuration
                      localId:_localId
                     remoteId:targetId
              signalingSender:self];
    [channel addObserver:self];
    [_peerConnectionChannels setObject:channel forKey:targetId];
  }
  return channel;
}
- (void)sendSignalingMessage:(NSString*)data
                          to:(NSString*)targetId
                   onSuccess:(void (^)())onSuccess
                   onFailure:(void (^)(NSError*))onFailure {
  [_signalingChannel sendMessage:data
                              to:targetId
                       onSuccess:onSuccess
                       onFailure:onFailure];
}
- (void)stop:(NSString*)targetId {
  if (![self checkSignalingChannelOnline:nil])
    return;
  OMSP2PPeerConnectionChannel* channel =
      [self getPeerConnectionChannel:targetId];
  [channel stopWithOnSuccess:nil onFailure:nil];
  [_peerConnectionChannels removeObjectForKey:targetId];
}
- (void)send:(NSString*)message
           to:(NSString*)targetId
    onSuccess:(nullable void (^)())onSuccess
    onFailure:(nullable void (^)(NSError*))onFailure {
  if (![self checkSignalingChannelOnline:onFailure])
    return;
  OMSP2PPeerConnectionChannel* channel =
      [self getPeerConnectionChannel:targetId];
  [channel send:message withOnSuccess:onSuccess onFailure:onFailure];
}
- (void)statsFor:(NSString*)targetId
       onSuccess:(void (^)(NSArray<RTCLegacyStatsReport*>*))onSuccess
       onFailure:(nullable void (^)(NSError*))onFailure {
  OMSP2PPeerConnectionChannel* channel =
      [self getPeerConnectionChannel:targetId];
  [channel statsWithOnSuccess:onSuccess onFailure:onFailure];
}
- (void)channel:(id<OMSP2PSignalingChannelProtocol>)channel
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
  OMSP2PPeerConnectionChannel* pcChannel =
      [self getPeerConnectionChannel:senderId];
  [pcChannel onIncomingSignalingMessage:message];
}
- (void)onInvitedFrom:(NSString*)remoteUserId {
  RTCLogInfo(@"On invited from %@", remoteUserId);
}
- (void)onStreamAdded:(OMSRemoteStream*)stream {
  RTCLogInfo(@"PeerClient received stream add.");
  if ([_delegate respondsToSelector:@selector(p2pClient:didAddStream:)]) {
    [_delegate p2pClient:self didAddStream:stream];
  }
}
- (void)onStreamRemoved:(OMSRemoteStream*)stream {
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
- (void)channelDidDisconnect:(id<OMSP2PSignalingChannelProtocol>)channel {
  RTCLogInfo(@"PeerClient received disconnect.");
  _peerClientState = kDisconnected;
  if ([_delegate respondsToSelector:@selector(p2pClientDidDisconnect:)]) {
    [_delegate p2pClientDidDisconnect:self];
  }
}
- (void)onStoppedFrom:(NSString*)remoteUserId {
  RTCLogInfo(@"PeerClient received chat stopped.");
}
- (void)onStartedFrom:(NSString*)remoteUserId {
  RTCLogInfo(@"PeerClient received chat started.");
}
@end
