//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/include/objc/ICS/ICSConferenceSubscribeOptions.h"
#import "talk/ics/sdk/include/cpp/ics/conference/subscribeoptions.h"

@interface ICSConferenceSubscribeOptions (Internal)

-(ics::conference::SubscribeOptions)nativeSubscribeOptions;

@end
