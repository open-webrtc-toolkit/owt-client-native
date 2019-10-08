// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <Foundation/Foundation.h>
#import <OWT/OWTClientConfiguration.h>
#import <OWT/OWTMediaFormat.h>
/**
 @brief Configuration for OWTP2PClient
 This configuration is used while creating OWTP2PClient. Changing this
 configuration does NOT impact existing OWTP2PClients.
 */
RTC_OBJC_EXPORT
@interface OWTP2PClientConfiguration : OWTClientConfiguration
@property(nonatomic, strong) NSArray<OWTAudioEncodingParameters*>* audio;
@property(nonatomic, strong) NSArray<OWTVideoEncodingParameters*>* video;
@end
