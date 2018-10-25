//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import "talk/oms/sdk/base/objc/OMSStream+Private.h"
#import "talk/oms/sdk/include/objc/OMS/OMSRemoteStream.h"

@interface OMSRemoteStream ()

- (std::shared_ptr<oms::base::RemoteStream>)nativeRemoteStream;

@end
