//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/include/objc/ICS/ICSRemoteStream.h"
#import "talk/ics/sdk/base/objc/ICSStream+Internal.h"

@interface ICSRemoteStream (Internal)

- (std::shared_ptr<ics::base::RemoteStream>)nativeRemoteStream;

@end
