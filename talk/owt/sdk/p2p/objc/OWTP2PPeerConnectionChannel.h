// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <Foundation/Foundation.h>
#import "RTCLegacyStatsReport.h"
#import "talk/owt/sdk/include/objc/OWT/OWTLocalStream.h"
#import "talk/owt/sdk/include/objc/OWT/OWTP2PSignalingSenderProtocol.h"
#import "talk/owt/sdk/include/objc/OWT/OWTP2PSignalingReceiverProtocol.h"
#import "talk/owt/sdk/include/objc/OWT/OWTP2PPeerConnectionChannelObserver.h"
#import "talk/owt/sdk/include/objc/OWT/OWTP2PClientConfiguration.h"
@class OWTP2PPublication;
@interface OWTP2PPeerConnectionChannel
    : NSObject<OWTP2PSignalingReceiverProtocol>
- (instancetype)initWithConfiguration:(OWTP2PClientConfiguration*)config
                              localId:(NSString*)localId
                             remoteId:(NSString*)remoteId
                      signalingSender:
                          (id<OWTP2PSignalingSenderProtocol>)signalingSender;
- (void)publish:(OWTLocalStream*)stream
      onSuccess:(void (^)(OWTP2PPublication*))onSuccess
      onFailure:(void (^)(NSError*))onFailure;
- (void)unpublish:(OWTLocalStream*)stream
        onSuccess:(void (^)(void))onSuccess
        onFailure:(void (^)(NSError*))onFailure;
- (void)send:(NSString*)message
    withOnSuccess:(void (^)(void))onSuccess
        onFailure:(void (^)(NSError*))onFailure;
- (void)stopWithOnSuccess:(void (^)(void))onSuccess
                onFailure:(void (^)(NSError*))onFailure;
- (void)statsWithOnSuccess:(void (^)(NSArray<RTCLegacyStatsReport*>*))onSuccess
                 onFailure:(void (^)(NSError*))onFailure;
- (void)statsForStream:(OWTStream*)stream
             onSuccess:(void (^)(NSArray<RTCLegacyStatsReport*>*))onSuccess
             onFailure:(void (^)(NSError*))onFailure;
- (void)addObserver:(id<OWTP2PPeerConnectionChannelObserver>)observer;
- (void)removeObserver:(id<OWTP2PPeerConnectionChannelObserver>)observer;
- (NSString*)getRemoteUserId;
@end
