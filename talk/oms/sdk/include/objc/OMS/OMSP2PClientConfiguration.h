// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <Foundation/Foundation.h>
#import <OMS/OMSClientConfiguration.h>
#import <OMS/OMSMediaFormat.h>
/**
 @brief Configuration for OMSP2PClient
 This configuration is used while creating OMSP2PClient. Changing this
 configuration does NOT impact existing OMSP2PClients.
 */
RTC_EXPORT
@interface OMSP2PClientConfiguration : OMSClientConfiguration
@property(nonatomic, strong) NSArray<OMSAudioEncodingParameters*>* audio;
@property(nonatomic, strong) NSArray<OMSVideoEncodingParameters*>* video;
@end
