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

typedef NS_ENUM(NSInteger, OMSAudioCodec) {
  OMSAudioCodecPcmu = 1,
  OMSAudioCodecPcma = 2,
  OMSAudioCodecOpus = 3,
  OMSAudioCodecG722 = 4,
  OMSAudioCodecIsac = 5,
  OMSAudioCodecIlbc = 6,
  OMSAudioCodecAac = 7,
  OMSAudioCodecAc3 = 8,
  OMSAudioCodecAsao = 9,
  OMSAudioCodecUnknown = 10,
};

typedef NS_ENUM(NSInteger, OMSVideoCodec) {
  OMSVideoCodecVP8 = 1,
  OMSVideoCodecVP9 = 2,
  OMSVideoCodecH264 = 3,
  OMSVideoCodecH265 = 4,
  OMSVideoCodecUnknown = 5,
};

typedef NS_ENUM(NSInteger, OMSAudioSourceInfo) {
  OMSAudioSourceInfoMic = 1,
  OMSAudioSourceInfoScreenCast = 2,
  OMSAudioSourceInfoFile = 3,
  OMSAudioSourceInfoMixed = 4,
  OMSAudioSourceInfoUnknown = 5,
};

typedef NS_ENUM(NSInteger, OMSVideoSourceInfo) {
  OMSVideoSourceInfoCamera = 1,
  OMSVideoSourceInfoScreenCast = 2,
  OMSVideoSourceInfoFile = 3,
  OMSVideoSourceInfoMixed = 4,
  OMSVideoSourceInfoUnknown = 5,
};

typedef NS_OPTIONS(NSInteger, OMSTrackKind) {
  OMSTrackKindUnknown = 0,
  OMSTrackKindAudio = 1 << 0,
  OMSTrackKindVideo = 1 << 1,
};

/// Codec parameters for an audio track.
RTC_EXPORT
@interface OMSAudioCodecParameters : NSObject

/**
 @brief Name of a codec.
 @details some functions do not support all the values in OMSAudioCodec.
*/
@property(nonatomic, assign) OMSAudioCodec name;
/// Numbers of channels for an audio track.
@property(nonatomic, assign) NSUInteger channelCount;
/// The codec clock rate expressed in Hertz.
@property(nonatomic, assign) NSUInteger clockRate;

@end

/// Codec parameters for a video track.
RTC_EXPORT
@interface OMSVideoCodecParameters : NSObject

/**
 @brief Name of a codec.
 @details some functions do not support all the values in OMSVideoCodec.
*/
@property(nonatomic, assign) OMSVideoCodec name;

/**
 @brief The profile of a codec.
 @details Profile may not apply to all codecs.
*/ 
@property(nonatomic, strong) NSString* profile;

@end

/// The audio settings of a publication.
RTC_EXPORT
@interface OMSAudioPublicationSettings : NSObject

@property(nonatomic, strong) OMSAudioCodecParameters* codec;

@end

/// The video settings of a publication.
RTC_EXPORT
@interface OMSVideoPublicationSettings : NSObject

@property(nonatomic, strong) OMSVideoCodecParameters* codec;
@property(nonatomic, assign) CGSize resolution;
@property(nonatomic, assign) CGFloat frameRate;
@property(nonatomic, assign) NSUInteger bitrate;
@property(nonatomic, assign) NSUInteger keyframeInterval;

@end

/// The settings of a publication.
RTC_EXPORT
@interface OMSPublicationSettings : NSObject

@property(nonatomic, strong) OMSAudioPublicationSettings* audio;
@property(nonatomic, strong) OMSVideoPublicationSettings* video;

@end

/// Represents the audio capability for subscription.
RTC_EXPORT
@interface OMSAudioSubscriptionCapabilities : NSObject

@property(nonatomic, strong) NSArray<OMSAudioCodecParameters*>* codecs;

@end

/// Represents the video capability for subscription.
RTC_EXPORT
@interface OMSVideoSubscriptionCapabilities : NSObject

@property(nonatomic, strong) NSArray<OMSVideoCodecParameters*>* codecs;
@property(nonatomic, strong) NSArray<NSValue*>* resolutions;
@property(nonatomic, strong) NSArray<NSNumber*>* frameRates;
@property(nonatomic, strong) NSArray<NSNumber*>* bitrateMultipliers;
@property(nonatomic, strong) NSArray<NSNumber*>* keyframeIntervals;

@end

/// Represents the capability for subscription.
RTC_EXPORT
@interface OMSSubscriptionCapabilities : NSObject

@property(nonatomic, strong) OMSAudioSubscriptionCapabilities* audio;
@property(nonatomic, strong) OMSVideoSubscriptionCapabilities* video;

@end

/// Information of a stream's source.
RTC_EXPORT
@interface OMSStreamSourceInfo : NSObject

@property(nonatomic, assign) OMSAudioSourceInfo audio;
@property(nonatomic, assign) OMSVideoSourceInfo video;

@end

RTC_EXPORT
/// Constraints for creating a video MediaStreamTrack.
@interface OMSVideoTrackConstraints : NSObject

@property(nonatomic, assign) double frameRate;
@property(nonatomic, assign) CGSize resolution;
@property(nonatomic, assign) AVCaptureDevicePosition devicePosition;

@end

/// Constraints for creating a MediaStream from screen mic and camera.
RTC_EXPORT
@interface OMSStreamConstraints : NSObject

/**
  @brief Indicate whether audio track is enabled.
  @details If audio is true, MediaStream created with this constraints object
  will capture audio from mic. Otherwise, audio will be disabled.
 */
@property(nonatomic, assign) BOOL audio;
/// Constraints for video track.
@property(nonatomic, strong, nullable) OMSVideoTrackConstraints* video;

@end

RTC_EXPORT
/// Encoding parameters for sending an audio track.
@interface OMSAudioEncodingParameters : NSObject

@property(nonatomic, strong) OMSAudioCodecParameters* codec;
/// Max bitrate expressed in bps.
@property(nonatomic, assign) NSUInteger maxBitrate;

@end

RTC_EXPORT
/// Encoding parameters for sending a video track.
@interface OMSVideoEncodingParameters : NSObject

@property(nonatomic, strong) OMSVideoCodecParameters* codec;
/// Max bitrate expressed in bps.
@property(nonatomic, assign) NSUInteger maxBitrate;

@end

NS_ASSUME_NONNULL_END
