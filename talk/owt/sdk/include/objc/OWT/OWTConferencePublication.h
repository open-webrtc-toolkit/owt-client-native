// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <Foundation/Foundation.h>
#import <WebRTC/RTCMacros.h>
#import <WebRTC/RTCLegacyStatsReport.h>
#import <OWT/OWTMediaFormat.h>
NS_ASSUME_NONNULL_BEGIN
@class OWTConferencePublication;
RTC_OBJC_EXPORT
@protocol OWTConferencePublicationDelegate <NSObject>
@optional
/// Publication is ended.
- (void)publicationDidEnd:(OWTConferencePublication*)publication;
/// Publication is muted. Client stopped sending audio and/or video data to remote endpoint.
- (void)publicationDidMute:(OWTConferencePublication*)publication
                  trackKind:(OWTTrackKind)kind;
/// Publication is unmuted. Client continued sending audio and/or video data to remote endpoint.
- (void)publicationDidUnmute:(OWTConferencePublication*)publication
                    trackKind:(OWTTrackKind)kind;
/// Publication encountered ICE failure or sever reported failure and cannot be used any more.
- (void)publicationDidError:(OWTConferencePublication*)publication
                    errorInfo:(NSError*)error;
@end
/**
  @brief Publication represents a sender for publishing a stream.
  @details It handles the actions on a LocalStream published to a conference.
*/
RTC_OBJC_EXPORT
@interface OWTConferencePublication : NSObject
- (instancetype)init NS_UNAVAILABLE;
/// Stop certain publication. Once a subscription is stopped, it cannot be recovered.
- (void)stop;
/// Stop sending data to remote endpoint.
- (void)mute:(OWTTrackKind)trackKind
    onSuccess:(nullable void (^)())onSuccess
    onFailure:(nullable void (^)(NSError*))onFailure;
/// Continue sending data to remote endpoint.
- (void)unmute:(OWTTrackKind)trackKind
     onSuccess:(nullable void (^)())onSuccess
     onFailure:(nullable void (^)(NSError*))onFailure;
/// Get stats of underlying PeerConnection.
- (void)statsWithOnSuccess:(void (^)(NSArray<RTCLegacyStatsReport*>*))onSuccess
                 onFailure:(nullable void (^)(NSError*))onFailure;
@property(nonatomic, strong, readonly) NSString* publicationId;
@property(nonatomic, weak) id<OWTConferencePublicationDelegate> delegate;
@end
NS_ASSUME_NONNULL_END
