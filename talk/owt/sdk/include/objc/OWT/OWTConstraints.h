// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <Foundation/Foundation.h>
#import <WebRTC/RTCMacros.h>
#import <OWT/IcsMediaCodec.h>
NS_ASSUME_NONNULL_BEGIN
/// The audio settings of a publication.
@interface OWTAudioPublicationSettings
@property(nonatomic, strong, readonly) OWTAudioCodecParameters* codec;
@end
/// The video settings of a publication.
@interface OWTVideoPublicationSettings
@end
/// The settings of a publication.
@interface OWTPublicationSettings
@property(nonatomic, strong, readonly) OWTAudioPublicationSettings* audio;
@property(nonatomic, strong, readonly) OWTVideoPublicationSettings* video;
@end
NS_ASSUME_NONNULL_END
