// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <AVFoundation/AVFoundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>
#import <WebRTC/RTCMacros.h>
NS_ASSUME_NONNULL_BEGIN
typedef NS_ENUM(NSInteger, OWTAudioCodec) {
  OWTAudioCodecPcmu = 1,
  OWTAudioCodecPcma = 2,
  OWTAudioCodecOpus = 3,
  OWTAudioCodecG722 = 4,
  OWTAudioCodecIsac = 5,
  OWTAudioCodecIlbc = 6,
  OWTAudioCodecAac = 7,
  OWTAudioCodecAc3 = 8,
  OWTAudioCodecAsao = 9,
  OWTAudioCodecUnknown = 10,
};
typedef NS_ENUM(NSInteger, OWTVideoCodec) {
  OWTVideoCodecVP8 = 1,
  OWTVideoCodecVP9 = 2,
  OWTVideoCodecH264 = 3,
  OWTVideoCodecH265 = 4,
  OWTVideoCodecUnknown = 5,
};
typedef NS_ENUM(NSInteger, OWTAudioSourceInfo) {
  OWTAudioSourceInfoMic = 1,
  OWTAudioSourceInfoScreenCast = 2,
  OWTAudioSourceInfoFile = 3,
  OWTAudioSourceInfoMixed = 4,
  OWTAudioSourceInfoUnknown = 5,
};
typedef NS_ENUM(NSInteger, OWTVideoSourceInfo) {
  OWTVideoSourceInfoCamera = 1,
  OWTVideoSourceInfoScreenCast = 2,
  OWTVideoSourceInfoFile = 3,
  OWTVideoSourceInfoMixed = 4,
  OWTVideoSourceInfoUnknown = 5,
};
typedef NS_OPTIONS(NSInteger, OWTTrackKind) {
  OWTTrackKindUnknown = 0,
  OWTTrackKindAudio = 1 << 0,
  OWTTrackKindVideo = 1 << 1,
};
/// Codec parameters for an audio track.
RTC_OBJC_EXPORT
@interface OWTAudioCodecParameters : NSObject
/**
 @brief Name of a codec.
 @details some functions do not support all the values in OWTAudioCodec.
*/
@property(nonatomic, assign) OWTAudioCodec name;
/// Numbers of channels for an audio track.
@property(nonatomic, assign) NSUInteger channelCount;
/// The codec clock rate expressed in Hertz.
@property(nonatomic, assign) NSUInteger clockRate;
@end
/// Codec parameters for a video track.
RTC_OBJC_EXPORT
@interface OWTVideoCodecParameters : NSObject
/**
 @brief Name of a codec.
 @details some functions do not support all the values in OWTVideoCodec.
*/
@property(nonatomic, assign) OWTVideoCodec name;
/**
 @brief The profile of a codec.
 @details Profile may not apply to all codecs.
*/
@property(nonatomic, strong) NSString* profile;
@end
/// The audio settings of a publication.
RTC_OBJC_EXPORT
@interface OWTAudioPublicationSettings : NSObject
@property(nonatomic, strong) OWTAudioCodecParameters* codec;
@end
/// The video settings of a publication.
RTC_OBJC_EXPORT
@interface OWTVideoPublicationSettings : NSObject
@property(nonatomic, strong) OWTVideoCodecParameters* codec;
@property(nonatomic, assign) CGSize resolution;
@property(nonatomic, assign) CGFloat frameRate;
@property(nonatomic, assign) NSUInteger bitrate;
@property(nonatomic, assign) NSUInteger keyframeInterval;
@property(nonatomic, strong) NSString* rid;
@end
/// The settings of a publication.
RTC_OBJC_EXPORT
@interface OWTPublicationSettings : NSObject
@property(nonatomic, strong) NSArray<OWTAudioPublicationSettings*>* audio;
@property(nonatomic, strong) NSArray<OWTVideoPublicationSettings*>* video;
@end
/// Represents the audio capability for subscription.
RTC_OBJC_EXPORT
@interface OWTAudioSubscriptionCapabilities : NSObject
@property(nonatomic, strong) NSArray<OWTAudioCodecParameters*>* codecs;
@end
/// Represents the video capability for subscription.
RTC_OBJC_EXPORT
@interface OWTVideoSubscriptionCapabilities : NSObject
@property(nonatomic, strong) NSArray<OWTVideoCodecParameters*>* codecs;
@property(nonatomic, strong) NSArray<NSValue*>* resolutions;
@property(nonatomic, strong) NSArray<NSNumber*>* frameRates;
@property(nonatomic, strong) NSArray<NSNumber*>* bitrateMultipliers;
@property(nonatomic, strong) NSArray<NSNumber*>* keyframeIntervals;
@end
/// Represents the capability for subscription.
RTC_OBJC_EXPORT
@interface OWTSubscriptionCapabilities : NSObject
@property(nonatomic, strong) OWTAudioSubscriptionCapabilities* audio;
@property(nonatomic, strong) OWTVideoSubscriptionCapabilities* video;
@end
/// Information of a stream's source.
RTC_OBJC_EXPORT
@interface OWTStreamSourceInfo : NSObject
@property(nonatomic, assign) OWTAudioSourceInfo audio;
@property(nonatomic, assign) OWTVideoSourceInfo video;
@end
RTC_OBJC_EXPORT
/// Constraints for creating a video MediaStreamTrack.
@interface OWTVideoTrackConstraints : NSObject
@property(nonatomic, assign) double frameRate;
@property(nonatomic, assign) CGSize resolution;
@property(nonatomic, assign) AVCaptureDevicePosition devicePosition;
@end
/// Constraints for creating a MediaStream from screen mic and camera.
RTC_OBJC_EXPORT
@interface OWTStreamConstraints : NSObject
/**
  @brief Indicate whether audio track is enabled.
  @details If audio is true, MediaStream created with this constraints object
  will capture audio from mic. Otherwise, audio will be disabled.
 */
@property(nonatomic, assign) BOOL audio;
/// Constraints for video track.
@property(nonatomic, strong, nullable) OWTVideoTrackConstraints* video;
@end
RTC_OBJC_EXPORT
/// Encoding parameters for sending an audio track.
@interface OWTAudioEncodingParameters : NSObject
@property(nonatomic, strong) OWTAudioCodecParameters* codec;
/// Max bitrate expressed in bps.
@property(nonatomic, assign) NSUInteger maxBitrate;
@end
RTC_OBJC_EXPORT
/// Encoding parameters for sending a video track.
@interface OWTVideoEncodingParameters : NSObject
@property(nonatomic, strong) OWTVideoCodecParameters* codec;
/// Max bitrate expressed in bps.
@property(nonatomic, assign) NSUInteger maxBitrate;
@end
NS_ASSUME_NONNULL_END
