//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import "talk/woogeen/sdk/base/objc/RTCStream+Internal.h"
#import "talk/woogeen/sdk/include/objc/Woogeen/RTCLocalStream.h"

@interface RTCLocalStream (Internal)

- (std::shared_ptr<woogeen::base::LocalStream>)nativeLocalStream;
- (NSString*)streamId;

@end
