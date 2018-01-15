//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/include/objc/Woogeen/RTCRemoteStream.h"
#import "talk/ics/sdk/base/objc/RTCStream+Internal.h"

@interface RTCRemoteStream (Internal)

- (std::shared_ptr<ics::base::RemoteStream>)nativeRemoteStream;

@end
