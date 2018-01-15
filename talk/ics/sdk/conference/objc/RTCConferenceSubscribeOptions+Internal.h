//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/include/objc/Woogeen/RTCConferenceSubscribeOptions.h"
#import "talk/ics/sdk/include/cpp/ics/conference/subscribeoptions.h"

@interface RTCConferenceSubscribeOptions (Internal)

-(ics::conference::SubscribeOptions)nativeSubscribeOptions;

@end
