//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/base/objc/ICSStream+Internal.h"
#import "talk/ics/sdk/include/objc/ICS/ICSLocalStream.h"

@interface ICSLocalStream (Internal)

- (std::shared_ptr<ics::base::LocalStream>)nativeLocalStream;
- (NSString*)streamId;

@end
