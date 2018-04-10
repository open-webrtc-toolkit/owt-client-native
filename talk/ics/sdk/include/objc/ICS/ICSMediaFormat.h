/*
 * Copyright © 2016 Intel Corporation. All Rights Reserved.
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

#import <AVFoundation/AVFoundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>
#import <WebRTC/RTCMacros.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSInteger, ICSAudioCodec) {
  ICSAudioCodecPcmu = 1,
  ICSAudioCodecPcma = 2,
  ICSAudioCodecOpus = 3,
  ICSAudioCodecG722 = 4,
  ICSAudioCodecIsac = 5,
  ICSAudioCodecIlbc = 6,
  ICSAudioCodecAac = 7,
  ICSAudioCodecAc3 = 8,
  ICSAudioCodecAsao = 9,
  ICSAudioCodecUnknown = 10,
};

typedef NS_ENUM(NSInteger, ICSVideoCodec) {
  ICSVideoCodecVP8 = 1,
  ICSVideoCodecVP9 = 2,
  ICSVideoCodecH264 = 3,
  ICSVideoCodecH265 = 4,
  ICSVideoCodecUnknown = 5,
};

typedef NS_ENUM(NSInteger, ICSAudioSourceInfo) {
  ICSAudioSourceInfoMic = 1,
  ICSAudioSourceInfoScreenCast = 2,
  ICSAudioSourceInfoFile = 3,
  ICSAudioSourceInfoMixed = 4,
  ICSAudioSourceInfoUnknown = 5,
};

typedef NS_ENUM(NSInteger, ICSVideoSourceInfo) {
  ICSVideoSourceInfoCamera = 1,
  ICSVideoSourceInfoScreenCast = 2,
  ICSVideoSourceInfoFile = 3,
  ICSVideoSourceInfoMixed = 4,
  ICSVideoSourceInfoUnknown = 5,
};

typedef NS_OPTIONS(NSInteger, ICSTrackKind) {
  ICSTrackKindUnknown = 0,
  ICSTrackKindAudio = 1 << 0,
  ICSTrackKindVideo = 1 << 1,
};

/// Codec parameters for an audio track.
RTC_EXPORT
@interface ICSAudioCodecParameters : NSObject

/**
 @brief Name of a codec.
 @details some functions do not support all the values in ICSAudioCodec.
*/
@property(nonatomic, assign) ICSAudioCodec name;
/// Numbers of channels for an audio track.
@property(nonatomic, assign) NSUInteger channelCount;
/// The codec clock rate expressed in Hertz.
@property(nonatomic, assign) NSUInteger clockRate;

@end

/// Codec parameters for a video track.
RTC_EXPORT
@interface ICSVideoCodecParameters : NSObject

/**
 @brief Name of a codec.
 @details some functions do not support all the values in ICSVideoCodec.
*/
@property(nonatomic, assign) ICSVideoCodec name;

/**
 @brief The profile of a codec.
 @details Profile may not apply to all codecs.
*/ 
@property(nonatomic, strong) NSString* profile;

@end

/// The audio settings of a publication.
RTC_EXPORT
@interface ICSAudioPublicationSettings : NSObject

@property(nonatomic, strong) ICSAudioCodecParameters* codec;

@end

/// The video settings of a publication.
RTC_EXPORT
@interface ICSVideoPublicationSettings : NSObject

@property(nonatomic, strong) ICSVideoCodecParameters* codec;
@property(nonatomic, assign) CGSize resolution;
@property(nonatomic, assign) CGFloat frameRate;
@property(nonatomic, assign) NSUInteger bitrate;
@property(nonatomic, assign) NSUInteger keyframeInterval;

@end

/// The settings of a publication.
RTC_EXPORT
@interface ICSPublicationSettings : NSObject

@property(nonatomic, strong) ICSAudioPublicationSettings* audio;
@property(nonatomic, strong) ICSVideoPublicationSettings* video;

@end

/// Represents the audio capability for subscription.
RTC_EXPORT
@interface ICSAudioSubscriptionCapabilities : NSObject

@property(nonatomic, strong) NSArray<ICSAudioCodecParameters*>* codecs;

@end

/// Represents the video capability for subscription.
RTC_EXPORT
@interface ICSVideoSubscriptionCapabilities : NSObject

@property(nonatomic, strong) NSArray<ICSVideoCodecParameters*>* codecs;
@property(nonatomic, strong) NSArray<NSValue*>* resolutions;
@property(nonatomic, strong) NSArray<NSNumber*>* frameRates;
@property(nonatomic, strong) NSArray<NSNumber*>* bitrateMultipliers;
@property(nonatomic, strong) NSArray<NSNumber*>* keyframeIntervals;

@end

/// Represents the capability for subscription.
RTC_EXPORT
@interface ICSSubscriptionCapabilities : NSObject

@property(nonatomic, strong) ICSAudioSubscriptionCapabilities* audio;
@property(nonatomic, strong) ICSVideoSubscriptionCapabilities* video;

@end

/// Information of a stream's source.
RTC_EXPORT
@interface ICSStreamSourceInfo : NSObject

@property(nonatomic, assign) ICSAudioSourceInfo audio;
@property(nonatomic, assign) ICSVideoSourceInfo video;

@end

RTC_EXPORT
/// Constraints for creating a video MediaStreamTrack.
@interface ICSVideoTrackConstraints : NSObject

@property(nonatomic, assign) double frameRate;
@property(nonatomic, assign) CGSize resolution;
@property(nonatomic, assign) AVCaptureDevicePosition devicePosition;

@end

/// Constraints for creating a MediaStream from screen mic and camera.
RTC_EXPORT
@interface ICSStreamConstraints : NSObject

/**
  @brief Indicate whether audio track is enabled.
  @details If audio is true, MediaStream created with this constraints object
  will capture audio from mic. Otherwise, audio will be disabled.
 */
@property(nonatomic, assign) BOOL audio;
/// Constraints for video track.
@property(nonatomic, strong, nullable) ICSVideoTrackConstraints* video;

@end

RTC_EXPORT
/// Encoding parameters for sending an audio track.
@interface ICSAudioEncodingParameters : NSObject

@property(nonatomic, strong) ICSAudioCodecParameters* codec;
/// Max bitrate expressed in bps.
@property(nonatomic, assign) NSUInteger maxBitrate;

@end

RTC_EXPORT
/// Encoding parameters for sending a video track.
@interface ICSVideoEncodingParameters : NSObject

@property(nonatomic, strong) ICSVideoCodecParameters* codec;
/// Max bitrate expressed in bps.
@property(nonatomic, assign) NSUInteger maxBitrate;

@end

NS_ASSUME_NONNULL_END
