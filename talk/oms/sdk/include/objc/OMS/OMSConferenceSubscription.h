// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <WebRTC/RTCMacros.h>
#import <OMS/OMSMediaFormat.h>
NS_ASSUME_NONNULL_BEGIN
@class RTCLegacyStatsReport;
@class OMSConferenceSubscription;
@class OMSConferenceSubscriptionUpdateOptions;
RTC_EXPORT
@protocol OMSConferenceSubscriptionDelegate <NSObject>
@optional
/// Subscription is ended.
- (void)subscriptionDidEnd:(OMSConferenceSubscription*)subscription;
/// Publication is muted. Remote side stopped sending audio and/or video data.
- (void)subscriptionDidMute:(OMSConferenceSubscription*)subscription
                  trackKind:(OMSTrackKind)kind;
/// Publication is unmuted. Remote side continued sending audio and/or video data.
- (void)subscriptionDidUnmute:(OMSConferenceSubscription*)subscription
                    trackKind:(OMSTrackKind)kind;
@end
RTC_EXPORT
@interface OMSConferenceSubscription : NSObject
- (instancetype)init NS_UNAVAILABLE;
/// Stop certain subscription. Once a subscription is stopped, it cannot be recovered.
- (void)stop;
/// Stop reeving data from remote endpoint.
- (void)mute:(OMSTrackKind)trackKind
    onSuccess:(nullable void (^)())onSuccess
    onFailure:(nullable void (^)(NSError*))onFailure;
/// Continue reeving data from remote endpoint.
- (void)unmute:(OMSTrackKind)trackKind
     onSuccess:(nullable void (^)())onSuccess
     onFailure:(nullable void (^)(NSError*))onFailure;
/// Get stats of underlying PeerConnection.
- (void)statsWithOnSuccess:(void (^)(NSArray<RTCLegacyStatsReport*>*))onSuccess
                 onFailure:(nullable void (^)(NSError*))onFailure;
/// Update subscription with given options.
- (void)applyOptions:(OMSConferenceSubscriptionUpdateOptions*)options
           onSuccess:(nullable void (^)())onSuccess
           onFailure:(nullable void (^)(NSError*))onFailure;
@property(nonatomic, strong, readonly) NSString* subscriptionId;
@property(nonatomic, weak) id<OMSConferenceSubscriptionDelegate> delegate;
@end

RTC_EXPORT
/// Constraints for subscribing a remote stream.
@interface OMSConferenceAudioSubscriptionConstraints : NSObject
@property(nonatomic, assign) BOOL disabled;
@property(nonatomic, strong) NSArray<OMSAudioCodecParameters*>* codecs;
@end
RTC_EXPORT
/// Constraints for subscribing a remote stream.
@interface OMSConferenceVideoSubscriptionConstraints : NSObject
@property(nonatomic, assign) BOOL disabled;
@property(nonatomic, assign) CGSize resolution;
@property(nonatomic, assign) double frameRate;
@property(nonatomic, assign) double bitrateMultiplier;
@property(nonatomic, assign) NSUInteger keyFrameInterval;
@property(nonatomic, strong) NSArray<OMSVideoCodecParameters*>* codecs;
@end
RTC_EXPORT
@interface OMSConferenceSubscribeOptions : NSObject
- (instancetype)initWithAudio:(OMSConferenceAudioSubscriptionConstraints*)audio
                        video:(OMSConferenceVideoSubscriptionConstraints*)video;
@property(nonatomic, strong) OMSConferenceAudioSubscriptionConstraints* audio;
@property(nonatomic, strong) OMSConferenceVideoSubscriptionConstraints* video;
@end
RTC_EXPORT
/// Constraints for updating a subscription.
@interface OMSConferenceVideoSubscriptionUpdateConstraints : NSObject
@property(nonatomic, assign) CGSize resolution;
@property(nonatomic, assign) double frameRate;
@property(nonatomic, assign) double bitrateMultiplier;
@property(nonatomic, assign) NSUInteger keyFrameInterval;
@end
RTC_EXPORT
@interface OMSConferenceSubscriptionUpdateOptions : NSObject
@property(nonatomic, strong)
    OMSConferenceVideoSubscriptionUpdateConstraints* video;
@end
NS_ASSUME_NONNULL_END
