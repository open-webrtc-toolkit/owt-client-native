// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OMS_CONFERENCE_OBJC_OMSCONFERENCECLIENTCONFIGURATION_H_
#define OMS_CONFERENCE_OBJC_OMSCONFERENCECLIENTCONFIGURATION_H_
#import <Foundation/Foundation.h>
#import <OMS/OMSClientConfiguration.h>
/// Configuration for creating a OMSConferenceClient
/**
  This configuration is used while creating OMSConferenceClient. Changing this
  configuration does NOT impact OMSConferenceClient already created.
*/
RTC_EXPORT
@interface OMSConferenceClientConfiguration : OMSClientConfiguration
@end
#endif  // OMS_CONFERENCE_OBJC_OMSCONFERENCECLIENTCONFIGURATION_H_
