//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#ifndef conference_ConferenceClient_h
#define conference_ConferenceClient_h

#import "talk/woogeen/sdk/base/objc/public/RTCLocalStream.h"
#import "talk/woogeen/sdk/base/objc/public/RTCRemoteStream.h"

@interface ConferenceClient : NSObject

-(void)join:(NSString*)token onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError*))onFailure;
-(void)publish:(RTCLocalStream*)stream onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError*))onFailure;
-(void)unpublish:(RTCLocalStream*)stream onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError*))onFailure;
-(void)subscribe:(RTCRemoteStream*)stream onSuccess:(void (^)(RTCRemoteStream*))onSuccess onFailure:(void (^)(NSError*))onFailure;
-(void)unsubscribe:(RTCRemoteStream*)stream onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError*))onFailure;
-(void)leaveWithOnSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError*))onFailure;

@end

#endif
