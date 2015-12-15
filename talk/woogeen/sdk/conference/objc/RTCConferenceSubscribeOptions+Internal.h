//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import "talk/woogeen/sdk/conference/objc/public/RTCConferenceSubscribeOptions.h"
#import "talk/woogeen/sdk/include/cpp/woogeen/subscribeoptions.h"

@interface RTCConferenceSubscribeOptions (Internal)

-(woogeen::SubscribeOptions)nativeSubscribeOptions;

@end
