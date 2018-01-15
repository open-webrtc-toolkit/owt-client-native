//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/base/objc/RTCStream+Internal.h"
#import "talk/ics/sdk/include/objc/Woogeen/RTCLocalStream.h"

@interface RTCLocalStream (Internal)

- (std::shared_ptr<ics::base::LocalStream>)nativeLocalStream;
- (NSString*)streamId;

@end
