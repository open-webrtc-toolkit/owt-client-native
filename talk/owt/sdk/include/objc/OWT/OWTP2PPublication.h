// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <Foundation/Foundation.h>
#import <WebRTC/RTCMacros.h>
NS_ASSUME_NONNULL_BEGIN
@class OWTP2PPublication;
RTC_OBJC_EXPORT
@protocol OWTP2PPublicationDelegate<NSObject>
@optional
/// Publication is ended.
- (void)publicationDidEnd:(OWTP2PPublication*)publication;
@end
/**
  @brief Publication represents a sender for publishing a stream.
  @details It handles the actions on a LocalStream published to a remote endpoint.
*/
RTC_OBJC_EXPORT
@interface OWTP2PPublication : NSObject
- (instancetype)init NS_UNAVAILABLE;
/// Stop certain publication. Once a subscription is stopped, it cannot be recovered.
- (void)stop;
/// Get stats of underlying PeerConnection.
- (void)stats:(void (^)(NSArray<RTCLegacyStatsReport*>* stats))onSuccess
    onFailure:(nullable void (^)(NSError*))onFailure;
@property(nonatomic, weak) id<OWTP2PPublicationDelegate> delegate;
@end
NS_ASSUME_NONNULL_END
