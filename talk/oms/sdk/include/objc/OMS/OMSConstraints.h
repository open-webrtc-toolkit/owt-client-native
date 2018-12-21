// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <Foundation/Foundation.h>
#import <WebRTC/RTCMacros.h>
#import <OMS/IcsMediaCodec.h>
NS_ASSUME_NONNULL_BEGIN
/// The audio settings of a publication.
@interface OMSAudioPublicationSettings
@property(nonatomic, strong, readonly) OMSAudioCodecParameters* codec;
@end
/// The video settings of a publication.
@interface OMSVideoPublicationSettings
@end
/// The settings of a publication.
@interface OMSPublicationSettings
@property(nonatomic, strong, readonly) OMSAudioPublicationSettings* audio;
@property(nonatomic, strong, readonly) OMSVideoPublicationSettings* video;
@end
NS_ASSUME_NONNULL_END
