//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/include/objc/ICS/ICSRemoteStream.h"
#import "talk/ics/sdk/base/objc/ICSStream+Private.h"

@interface ICSRemoteStream ()

- (std::shared_ptr<ics::base::RemoteStream>)nativeRemoteStream;

@end
