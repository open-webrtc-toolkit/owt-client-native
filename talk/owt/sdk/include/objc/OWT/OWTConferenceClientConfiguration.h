// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_CONFERENCE_OBJC_OWTCONFERENCECLIENTCONFIGURATION_H_
#define OWT_CONFERENCE_OBJC_OWTCONFERENCECLIENTCONFIGURATION_H_
#import <Foundation/Foundation.h>
#import <OWT/OWTClientConfiguration.h>
/// Configuration for creating a OWTConferenceClient
/**
  This configuration is used while creating OWTConferenceClient. Changing this
  configuration does NOT impact OWTConferenceClient already created.
*/
RTC_OBJC_EXPORT
@interface OWTConferenceClientConfiguration : OWTClientConfiguration
@end
#endif  // OWT_CONFERENCE_OBJC_OWTCONFERENCECLIENTCONFIGURATION_H_
