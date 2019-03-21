// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <Foundation/Foundation.h>
extern NSString* const OWTErrorDomain;
typedef NS_ENUM(NSInteger, OWTStreamErrors) {
  OWTStreamErrorUnknown = 1100,
  OWTStreamErrorLocalDeviceNotFound = 1102,
  OWTStreamErrorLocalInvalidOption = 1104,
  OWTStreamErrorLocalNotSupported = 1105
};
