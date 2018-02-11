//
//  Copyright (c) 2018 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/include/objc/ICS/ICSConferenceInfo.h"

#include "talk/ics/sdk/include/cpp/ics/conference/conferenceclient.h"

@interface ICSConferenceParticipant ()

@property(nonatomic, readonly) std::shared_ptr<const ics::conference::ConferenceInfo> nativeInfo;

- (instancetype)initWithNativeInfo:
    (std::shared_ptr<const ics::conference::ConferenceInfo>)info;

@end
