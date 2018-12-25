// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "talk/oms/sdk/include/objc/OMS/OMSConferenceClient.h"
@interface OMSConferenceClient (Internal)
/// Returns a published stream with specific ID.
- (OMSLocalStream*)publishedStreamWithId:(NSString*)streamId;
@end
