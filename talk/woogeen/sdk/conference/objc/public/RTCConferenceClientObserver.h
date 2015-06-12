//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#ifndef WOOGEEN_CONFERENCE_OBJC_RTCCONFERENCECLIENTOBSERVER_H_
#define WOOGEEN_CONFERENCE_OBJC_RTCCONFERENCECLIENTOBSERVER_H_

#import "RTCLocalStream.h"
#import "RTCRemoteStream.h"

@protocol RTCConferenceClientObserver <NSObject>

-(void)onServerDisconnected;
-(void)onStreamAdded:(RTCRemoteStream*)stream;
-(void)onStreamRemoved:(RTCRemoteStream*)stream;
// TODO(jianjun): Enable user events
//-(void)onUserJoined:(User*)user;
//-(void)onUserLeft:(User*)user;

@end

#endif  // WOOGEEN_CONFERENCE_OBJC_RTCCONFERENCECLIENTOBSERVER_H_
