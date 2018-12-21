// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <Foundation/Foundation.h>
extern NSString* const OMSErrorDomain;
typedef NS_ENUM(NSInteger, OMSStreamErrors) {
  OMSStreamErrorUnknown = 1100,
  OMSStreamErrorLocalDeviceNotFound = 1102,
  OMSStreamErrorLocalInvalidOption = 1104,
  OMSStreamErrorLocalNotSupported = 1105
};
