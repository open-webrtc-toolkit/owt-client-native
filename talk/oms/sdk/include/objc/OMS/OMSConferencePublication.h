// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <Foundation/Foundation.h>
#import <WebRTC/RTCMacros.h>
#import <WebRTC/RTCLegacyStatsReport.h>
#import <OMS/OMSMediaFormat.h>
NS_ASSUME_NONNULL_BEGIN
@class OMSConferencePublication;
RTC_EXPORT
@protocol OMSConferencePublicationDelegate <NSObject>
@optional
/// Publication is ended.
- (void)publicationDidEnd:(OMSConferencePublication*)publication;
/// Publication is muted. Client stopped sending audio and/or video data to remote endpoint.
- (void)publicationDidMute:(OMSConferencePublication*)publication
                  trackKind:(OMSTrackKind)kind;
/// Publication is unmuted. Client continued sending audio and/or video data to remote endpoint.
- (void)publicationDidUnmute:(OMSConferencePublication*)publication
                    trackKind:(OMSTrackKind)kind;
@end
/**
  @brief Publication represents a sender for publishing a stream.
  @details It handles the actions on a LocalStream published to a conference.
*/
RTC_EXPORT
@interface OMSConferencePublication : NSObject
- (instancetype)init NS_UNAVAILABLE;
/// Stop certain publication. Once a subscription is stopped, it cannot be recovered.
- (void)stop;
/// Stop sending data to remote endpoint.
- (void)mute:(OMSTrackKind)trackKind
    onSuccess:(nullable void (^)())onSuccess
    onFailure:(nullable void (^)(NSError*))onFailure;
/// Continue sending data to remote endpoint.
- (void)unmute:(OMSTrackKind)trackKind
     onSuccess:(nullable void (^)())onSuccess
     onFailure:(nullable void (^)(NSError*))onFailure;
/// Get stats of underlying PeerConnection.
- (void)statsWithOnSuccess:(void (^)(NSArray<RTCLegacyStatsReport*>*))onSuccess
                 onFailure:(nullable void (^)(NSError*))onFailure;
@property(nonatomic, strong, readonly) NSString* publicationId;
@property(nonatomic, weak) id<OMSConferencePublicationDelegate> delegate;
@end
NS_ASSUME_NONNULL_END
