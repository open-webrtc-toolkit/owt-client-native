//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import "talk/woogeen/sdk/include/objc/Woogeen/RTCPeerClient.h"
#import "talk/woogeen/sdk/include/objc/Woogeen/RTCP2PErrors.h"
#import "talk/woogeen/sdk/p2p/objc/RTCP2PPeerConnectionChannel.h"

@interface RTCPeerClient ()

- (RTCP2PPeerConnectionChannel*)getPeerConnectionChannel:(NSString*)targetId;

@end

typedef enum { kDisconnected, kConnecting, kConnected } SignalingChannelState;

@implementation RTCPeerClient {
  id<RTCP2PSignalingChannelProtocol> _signalingChannel;
  SignalingChannelState _peerClientState;
  NSMutableDictionary* _peerConnectionChannels;
  NSMutableArray* _observers;
  NSString* _localId;
  RTCPeerClientConfiguration* _configuration;
}

- (instancetype)initWithConfiguration:(RTCPeerClientConfiguration*)configuration
                     signalingChannel:
                         (id<RTCP2PSignalingChannelProtocol>)signalingChannel {
  self = [super init];
  _signalingChannel = signalingChannel;
  [_signalingChannel addObserver:self];
  _peerConnectionChannels = [[NSMutableDictionary alloc] init];
  _observers = [[NSMutableArray alloc] init];
  _configuration = configuration;
  _peerClientState = kDisconnected;
  return self;
}

- (void)addObserver:(id<RTCPeerClientObserver>)observer {
  [_observers addObject:observer];
}

- (void)removeObserver:(id<RTCPeerClientObserver>)observer {
  [_observers removeObject:observer];
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
    RTCP2PPeerConnectionChannel* channel =
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
          initWithDomain:RTCErrorDomain
                    code:WoogeenP2PErrorClientInvalidState
                userInfo:[[NSDictionary alloc]
                             initWithObjectsAndKeys:@"PeerClient haven't "
                                                    @"connect to a signaling "
                                                    @"server.",
                                                    NSLocalizedDescriptionKey,
                                                    nil]];
      failure(err);
    }
    return NO;
  }
  return YES;
}

- (void)invite:(NSString*)targetId
     onSuccess:(void (^)())onSuccess
     onFailure:(void (^)(NSError*))onFailure {
  if (![self checkSignalingChannelOnline:onFailure])
    return;
  RTCP2PPeerConnectionChannel* channel =
      [self getPeerConnectionChannel:targetId];
  [channel inviteWithOnSuccess:onSuccess onFailure:onFailure];
}

- (void)publish:(RTCLocalStream*)stream
             to:(NSString*)targetId
      onSuccess:(void (^)())onSuccess
      onFailure:(void (^)(NSError*))onFailure {
  if (![self checkSignalingChannelOnline:onFailure])
    return;
  RTCP2PPeerConnectionChannel* channel =
      [self getPeerConnectionChannel:targetId];
  [channel publish:stream onSuccess:onSuccess onFailure:onFailure];
}

- (void)unpublish:(RTCLocalStream*)stream
               to:(NSString*)targetId
        onSuccess:(void (^)())onSuccess
        onFailure:(void (^)(NSError*))onFailure {
  if (![self checkSignalingChannelOnline:onFailure])
    return;
  RTCP2PPeerConnectionChannel* channel =
      [self getPeerConnectionChannel:targetId];
  [channel unpublish:stream onSuccess:onSuccess onFailure:onFailure];
}

- (void)getConnectionStats:(NSString*)targetId
                 onSuccess:(void (^)(RTCConnectionStats*))onSuccess
                 onFailure:(void (^)(NSError*))onFailure {
  RTCP2PPeerConnectionChannel* channel =
      [self getPeerConnectionChannel:targetId];
  [channel getConnectionStatsWithOnSuccess:onSuccess onFailure:onFailure];
}

- (RTCP2PPeerConnectionChannel*)getPeerConnectionChannel:(NSString*)targetId {
  RTCP2PPeerConnectionChannel* channel =
      [_peerConnectionChannels objectForKey:targetId];
  if (channel == nil) {
    channel = [[RTCP2PPeerConnectionChannel alloc]
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

- (void)stop:(NSString*)targetId
   onSuccess:(void (^)())onSuccess
   onFailure:(void (^)(NSError*))onFailure {
  if (![self checkSignalingChannelOnline:onFailure])
    return;
  RTCP2PPeerConnectionChannel* channel =
      [self getPeerConnectionChannel:targetId];
  [channel stopWithOnSuccess:onSuccess onFailure:onFailure];
  [_peerConnectionChannels removeObjectForKey:targetId];
}

- (void)deny:(NSString*)targetId
   onSuccess:(void (^)())onSuccess
   onFailure:(void (^)(NSError*))onFailure {
  if (![self checkSignalingChannelOnline:onFailure])
    return;
  RTCP2PPeerConnectionChannel* channel =
      [self getPeerConnectionChannel:targetId];
  [channel denyWithOnSuccess:onSuccess onFailure:onFailure];
}

- (void)accept:(NSString*)targetId
     onSuccess:(void (^)())onSuccess
     onFailure:(void (^)(NSError*))onFailure {
  if (![self checkSignalingChannelOnline:onFailure])
    return;
  RTCP2PPeerConnectionChannel* channel =
      [self getPeerConnectionChannel:targetId];
  [channel acceptWithOnSuccess:onSuccess onFailure:onFailure];
}

- (void)send:(NSString*)targetId
     message:(NSString*)message
   onSuccess:(void (^)())onSuccess
   onFailure:(void (^)(NSError*))onFailure {
  if (![self checkSignalingChannelOnline:onFailure])
    return;
  RTCP2PPeerConnectionChannel* channel =
      [self getPeerConnectionChannel:targetId];
  [channel send:message withOnSuccess:onSuccess onFailure:onFailure];
}

- (void)onMessage:(NSString*)message from:(NSString*)senderId {
  RTCP2PPeerConnectionChannel* channel =
      [self getPeerConnectionChannel:senderId];
  [channel onIncomingSignalingMessage:message];
}

- (void)onInvitedFrom:(NSString*)remoteUserId {
  NSLog(@"On invited from %@", remoteUserId);
  for (id<RTCPeerClientObserver> observer in _observers) {
    [observer onInvited:remoteUserId];
  }
}

- (void)onStreamAdded:(RTCRemoteStream*)stream {
  NSLog(@"PeerClient received stream add.");
  for (id<RTCPeerClientObserver> observer in _observers) {
    [observer onStreamAdded:stream];
  }
}

- (void)onStreamRemoved:(RTCRemoteStream*)stream {
  NSLog(@"PeerClient received stream removed.");
  for (id<RTCPeerClientObserver> observer in _observers) {
    [observer onStreamRemoved:stream];
  }
}

- (void)onAcceptedFrom:(NSString*)remoteUserId {
  NSLog(@"PeerClient received accepted.");
  for (id<RTCPeerClientObserver> observer in _observers) {
    [observer onAccepted:remoteUserId];
  }
}

- (void)onDeniedFrom:(NSString*)remoteUserId {
  NSLog(@"PeerClient received Denied.");
  [_peerConnectionChannels removeObjectForKey: remoteUserId];
  for (id<RTCPeerClientObserver> observer in _observers) {
    [observer onDenied:remoteUserId];
  }
}

- (void)onDataReceivedFrom:(NSString*)remoteUserId withData:(NSString*)data {
  NSLog(@"Received data from data channel.");
  for (id<RTCPeerClientObserver> observer in _observers) {
    [observer onDataReceived:remoteUserId message:data];
  }
}

- (void)onDisconnected {
  NSLog(@"PeerClient received disconnect.");
  for (id<RTCPeerClientObserver> observer in _observers) {
    [observer onServerDisconnected];
  }
}

- (void)onStoppedFrom:(NSString*)remoteUserId {
  NSLog(@"PeerClient received chat stopped.");
  [_peerConnectionChannels removeObjectForKey: remoteUserId];
  for (id<RTCPeerClientObserver> observer in _observers) {
    [observer onChatStopped:remoteUserId];
  }
}

- (void)onStartedFrom:(NSString*)remoteUserId {
  NSLog(@"PeerClient received chat started.");
  for (id<RTCPeerClientObserver> observer in _observers) {
    [observer onChatStarted:remoteUserId];
  }
}

@end
