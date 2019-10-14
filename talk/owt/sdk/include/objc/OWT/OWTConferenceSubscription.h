// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <WebRTC/RTCMacros.h>
#import <OWT/OWTMediaFormat.h>
NS_ASSUME_NONNULL_BEGIN
@class RTCLegacyStatsReport;
@class OWTConferenceSubscription;
@class OWTConferenceSubscriptionUpdateOptions;
RTC_OBJC_EXPORT
@protocol OWTConferenceSubscriptionDelegate <NSObject>
@optional
/// Subscription is ended.
- (void)subscriptionDidEnd:(OWTConferenceSubscription*)subscription;
/// Subscription is muted. Remote side stopped sending audio and/or video data.
- (void)subscriptionDidMute:(OWTConferenceSubscription*)subscription
                  trackKind:(OWTTrackKind)kind;
/// Subscription is unmuted. Remote side continued sending audio and/or video data.
- (void)subscriptionDidUnmute:(OWTConferenceSubscription*)subscription
                    trackKind:(OWTTrackKind)kind;
/// Subscription encountered an ICE failure or server failure, and cannot be used anymore.
- (void)subscriptionDidError:(OWTConferenceSubscription*)subscription
                    errorInfo:(NSError*)error;
@end
RTC_OBJC_EXPORT
@interface OWTConferenceSubscription : NSObject
- (instancetype)init NS_UNAVAILABLE;
/// Stop certain subscription. Once a subscription is stopped, it cannot be recovered.
- (void)stop;
/// Stop reeving data from remote endpoint.
- (void)mute:(OWTTrackKind)trackKind
    onSuccess:(nullable void (^)())onSuccess
    onFailure:(nullable void (^)(NSError*))onFailure;
/// Continue reeving data from remote endpoint.
- (void)unmute:(OWTTrackKind)trackKind
     onSuccess:(nullable void (^)())onSuccess
     onFailure:(nullable void (^)(NSError*))onFailure;
/// Get stats of underlying PeerConnection.
- (void)statsWithOnSuccess:(void (^)(NSArray<RTCLegacyStatsReport*>*))onSuccess
                 onFailure:(nullable void (^)(NSError*))onFailure;
/// Update subscription with given options.
- (void)applyOptions:(OWTConferenceSubscriptionUpdateOptions*)options
           onSuccess:(nullable void (^)())onSuccess
           onFailure:(nullable void (^)(NSError*))onFailure;
@property(nonatomic, strong, readonly) NSString* subscriptionId;
@property(nonatomic, weak) id<OWTConferenceSubscriptionDelegate> delegate;
@end

RTC_OBJC_EXPORT
/// Constraints for subscribing a remote stream.
@interface OWTConferenceAudioSubscriptionConstraints : NSObject
@property(nonatomic, assign) BOOL disabled;
@property(nonatomic, strong) NSArray<OWTAudioCodecParameters*>* codecs;
@end

RTC_OBJC_EXPORT
/// Constraints for subscribing a remote stream.
@interface OWTConferenceVideoSubscriptionConstraints : NSObject
@property(nonatomic, assign) BOOL disabled;
@property(nonatomic, assign) CGSize resolution;
@property(nonatomic, assign) double frameRate;
@property(nonatomic, assign) double bitrateMultiplier;
@property(nonatomic, assign) NSUInteger keyFrameInterval;
@property(nonatomic, strong) NSArray<OWTVideoCodecParameters*>* codecs;
/// Restriction identifier to identify the RTP Streams within an RTP session. When rid is specified, other constraints will be ignored.
@property(nonatomic, strong) NSString* rid;
@end

RTC_OBJC_EXPORT
@interface OWTConferenceSubscribeOptions : NSObject
- (instancetype)initWithAudio:(OWTConferenceAudioSubscriptionConstraints*)audio
                        video:(OWTConferenceVideoSubscriptionConstraints*)video;
@property(nonatomic, strong) OWTConferenceAudioSubscriptionConstraints* audio;
@property(nonatomic, strong) OWTConferenceVideoSubscriptionConstraints* video;
@end
RTC_OBJC_EXPORT
/// Constraints for updating a subscription.
@interface OWTConferenceVideoSubscriptionUpdateConstraints : NSObject
@property(nonatomic, assign) CGSize resolution;
@property(nonatomic, assign) double frameRate;
@property(nonatomic, assign) double bitrateMultiplier;
@property(nonatomic, assign) NSUInteger keyFrameInterval;
@end
RTC_OBJC_EXPORT
@interface OWTConferenceSubscriptionUpdateOptions : NSObject
@property(nonatomic, strong)
    OWTConferenceVideoSubscriptionUpdateConstraints* video;
@end
NS_ASSUME_NONNULL_END
