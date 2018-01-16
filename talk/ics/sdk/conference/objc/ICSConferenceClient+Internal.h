//
//  Copyright (c) 2017 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/include/objc/ICS/ICSConferenceClient.h"

@interface ICSConferenceClient (Internal)

/// Returns a published stream with specific ID.
- (ICSLocalStream*)publishedStreamWithId:(NSString*)streamId;

@end
