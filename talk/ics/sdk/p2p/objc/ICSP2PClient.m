//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import <WebRTC/RTCLogging.h>

#import "talk/ics/sdk/include/objc/ICS/ICSP2PClient.h"
#import "talk/ics/sdk/include/objc/ICS/ICSP2PErrors.h"
#import "talk/ics/sdk/p2p/objc/ICSP2PPeerConnectionChannel.h"

@interface ICSP2PClient ()

- (ICSP2PPeerConnectionChannel*)getPeerConnectionChannel:(NSString*)targetId;

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
- (void)unpublish:(ICSLocalStream*)stream
               to:(NSString*)targetId
        onSuccess:(nullable void (^)())onSuccess
        onFailure:(nullable void (^)(NSError*))onFailure;

@end

typedef enum { kDisconnected, kConnecting, kConnected } SignalingChannelState;

@implementation ICSP2PClient {
  id<ICSP2PSignalingChannelProtocol> _signalingChannel;
  SignalingChannelState _peerClientState;
  NSMutableDictionary* _peerConnectionChannels;
  NSString* _localId;
  ICSP2PClientConfiguration* _configuration;
}

- (instancetype)initWithConfiguration:(ICSP2PClientConfiguration*)configuration
                signalingChannel:
                    (id<ICSP2PSignalingChannelProtocol>)signalingChannel {
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
    ICSP2PPeerConnectionChannel* channel =
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
          initWithDomain:ICSErrorDomain
                    code:ICSP2PErrorClientInvalidState
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

- (void)publish:(ICSLocalStream*)stream
             to:(NSString*)targetId
      onSuccess:(void (^)(ICSP2PPublication*))onSuccess
      onFailure:(void (^)(NSError*))onFailure {
  if (![self checkSignalingChannelOnline:onFailure])
    return;
  ICSP2PPeerConnectionChannel* channel =
      [self getPeerConnectionChannel:targetId];
  [channel publish:stream onSuccess:onSuccess onFailure:onFailure];
}

- (void)unpublish:(ICSLocalStream*)stream
               to:(NSString*)targetId
        onSuccess:(void (^)())onSuccess
        onFailure:(void (^)(NSError*))onFailure {
  if (![self checkSignalingChannelOnline:onFailure])
    return;
  ICSP2PPeerConnectionChannel* channel =
      [self getPeerConnectionChannel:targetId];
  [channel unpublish:stream onSuccess:onSuccess onFailure:onFailure];
}

- (ICSP2PPeerConnectionChannel*)getPeerConnectionChannel:(NSString*)targetId {
  ICSP2PPeerConnectionChannel* channel =
      [_peerConnectionChannels objectForKey:targetId];
  if (channel == nil) {
    channel = [[ICSP2PPeerConnectionChannel alloc]
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
  ICSP2PPeerConnectionChannel* channel =
      [self getPeerConnectionChannel:targetId];
  [channel stopWithOnSuccess:nil onFailure:nil];
  [_peerConnectionChannels removeObjectForKey:targetId];
}

- (void)send:(NSString*)targetId
     message:(NSString*)message
   onSuccess:(void (^)())onSuccess
   onFailure:(void (^)(NSError*))onFailure {
  if (![self checkSignalingChannelOnline:onFailure])
    return;
  ICSP2PPeerConnectionChannel* channel =
      [self getPeerConnectionChannel:targetId];
  [channel send:message withOnSuccess:onSuccess onFailure:onFailure];
}

- (void)statsFor:(NSString*)targetId
       onSuccess:(void (^)(NSArray<RTCLegacyStatsReport*>*))onSuccess
       onFailure:(nullable void (^)(NSError*))onFailure {
  ICSP2PPeerConnectionChannel* channel =
      [self getPeerConnectionChannel:targetId];
  [channel statsWithOnSuccess:onSuccess onFailure:onFailure];
}

- (void)channel:(id<ICSP2PSignalingChannelProtocol>)channel
    didReceiveMessage:(NSString*)message
                 from:(NSString*)senderId {
  if (![self.allowedRemoteIds containsObject:senderId]) {
    RTCLogInfo(@"Receive signaling message from disallowed user: %@.",
               senderId);
    return;
  }
  ICSP2PPeerConnectionChannel* pcChannel =
      [self getPeerConnectionChannel:senderId];
  [pcChannel onIncomingSignalingMessage:message];
}

- (void)onInvitedFrom:(NSString*)remoteUserId {
  RTCLogInfo(@"On invited from %@", remoteUserId);
}

- (void)onStreamAdded:(ICSRemoteStream*)stream {
  RTCLogInfo(@"PeerClient received stream add.");
  if ([_delegate respondsToSelector:@selector(p2pClient:didAddStream:)]) {
    [_delegate p2pClient:self didAddStream:stream];
  }
}

- (void)onStreamRemoved:(ICSRemoteStream*)stream {
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

- (void)channelDidDisconnect:(id<ICSP2PSignalingChannelProtocol>)channel {
  RTCLogInfo(@"PeerClient received disconnect.");
}

- (void)onStoppedFrom:(NSString*)remoteUserId {
  RTCLogInfo(@"PeerClient received chat stopped.");
}

- (void)onStartedFrom:(NSString*)remoteUserId {
  RTCLogInfo(@"PeerClient received chat started.");
}

@end
