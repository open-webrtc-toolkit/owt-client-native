//
//  Copyright (c) 2017 Intel Corporation. All rights reserved.
//

#import "talk/woogeen/sdk/include/objc/Woogeen/RTCConferenceClient.h"

@interface RTCConferenceClient (Internal)

/// Returns a published stream with specific ID.
- (RTCLocalStream*)publishedStreamWithId:(NSString*)streamId;

@end
