/*
 * Copyright © 2018 Intel Corporation. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
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
