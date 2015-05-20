/*
 * Intel License
 */

#import <Foundation/Foundation.h>
#import "talk/woogeen/sdk/base/objc/public/RTCRemoteStream.h"

// RTCP2PPeerConnectionChannelObserver is an ObjectiveC wrapper for P2PPeerConnectionChannelObserver.
@protocol RTCP2PPeerConnectionChannelObserver <NSObject>

- (void)onInvitedFrom:(NSString*)remoteUserId;
- (void)onAcceptedFrom:(NSString*)remoteUserId;
- (void)onDeniedFrom:(NSString*)remoteUserId;
- (void)onChatStoppedFrom:(NSString*)remoteUserId;
- (void)onChatStartedFrom:(NSString*)remoteUserId;
- (void)onDataReceivedFrom:(NSString*)remoteUserId withData:(NSString*)data;
- (void)onStreamAdded:(RTCRemoteStream*)stream;
- (void)onStreamRemoved:(RTCRemoteStream*)stream;

@end
