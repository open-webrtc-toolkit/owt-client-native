// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "talk/owt/sdk/include/objc/OWT/OWTConferenceClient.h"
@interface OWTConferenceClient (Internal)
/// Returns a published stream with specific ID.
- (OWTLocalStream*)publishedStreamWithId:(NSString*)streamId;
@end
