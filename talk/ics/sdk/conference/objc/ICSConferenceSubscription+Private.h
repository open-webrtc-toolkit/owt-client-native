//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/include/cpp/ics/conference/conferencesubscription.h"
#import "talk/ics/sdk/include/cpp/ics/conference/subscribeoptions.h"
#import "talk/ics/sdk/include/objc/ICS/ICSConferenceSubscription.h"

@interface ICSConferenceSubscription ()

- (instancetype)initWithNativeSubscription:
    (std::shared_ptr<ics::conference::ConferenceSubscription>)
        nativeSubscription;

@end

@interface ICSConferenceAudioSubscriptionConstraints ()

- (std::shared_ptr<ics::conference::AudioSubscriptionConstraints>)
    nativeAudioSubscriptionConstraints;

@end

@interface ICSConferenceVideoSubscriptionConstraints ()

- (std::shared_ptr<ics::conference::VideoSubscriptionConstraints>)
    nativeVideoSubscriptionConstraints;

@end

@interface ICSConferenceSubscribeOptions ()

- (std::shared_ptr<ics::conference::SubscribeOptions>)nativeSubscribeOptions;

@end

@interface ICSConferenceVideoSubscriptionUpdateConstraints ()

- (std::shared_ptr<ics::conference::VideoSubscriptionUpdateConstraints>)
    nativeVideoSubscriptionUpdateConstraints;

@end

@interface ICSConferenceSubscriptionUpdateOptions ()

- (std::shared_ptr<ics::conference::SubscriptionUpdateOptions>)
    nativeSubscriptionUpdateOptions;

@end
