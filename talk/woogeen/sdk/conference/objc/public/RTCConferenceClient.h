//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#ifndef WOOGEEN_CONFERENCE_OBJC_RTCCONFERENCECLIENT_H_
#define WOOGEEN_CONFERENCE_OBJC_RTCCONFERENCECLIENT_H_

#import "talk/woogeen/sdk/base/objc/public/RTCLocalStream.h"
#import "talk/woogeen/sdk/base/objc/public/RTCRemoteStream.h"
#import "talk/woogeen/sdk/conference/objc/public/RTCConferenceClientObserver.h"

@interface RTCConferenceClient : NSObject

-(void)join:(NSString*)token onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError*))onFailure;
-(void)publish:(RTCLocalStream*)stream onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError*))onFailure;
-(void)unpublish:(RTCLocalStream*)stream onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError*))onFailure;
-(void)subscribe:(RTCRemoteStream*)stream onSuccess:(void (^)(RTCRemoteStream*))onSuccess onFailure:(void (^)(NSError*))onFailure;
-(void)unsubscribe:(RTCRemoteStream*)stream onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError*))onFailure;
-(void)leaveWithOnSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError*))onFailure;
-(void)addObserver:(id<RTCConferenceClientObserver>)observer;
-(void)removeObserver:(id<RTCConferenceClientObserver>)observer;

@end

#endif  // WOOGEEN_CONFERENCE_OBJC_RTCCONFERENCECLIENT_H_
