//
//  Copyright (c) 2017 Intel Corporation. All rights reserved.
//
#import "talk/oms/sdk/include/objc/OMS/OMSConferenceClient.h"
@interface OMSConferenceClient (Internal)
/// Returns a published stream with specific ID.
- (OMSLocalStream*)publishedStreamWithId:(NSString*)streamId;
@end
