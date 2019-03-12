// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "talk/owt/sdk/include/cpp/owt/conference/conferencesubscription.h"
#import "talk/owt/sdk/include/cpp/owt/conference/subscribeoptions.h"
#import "talk/owt/sdk/include/objc/OWT/OWTConferenceSubscription.h"
@interface OWTConferenceSubscription ()
- (instancetype)initWithNativeSubscription:
    (std::shared_ptr<owt::conference::ConferenceSubscription>)
        nativeSubscription;
@end
@interface OWTConferenceAudioSubscriptionConstraints ()
- (std::shared_ptr<owt::conference::AudioSubscriptionConstraints>)
    nativeAudioSubscriptionConstraints;
@end
@interface OWTConferenceVideoSubscriptionConstraints ()
- (std::shared_ptr<owt::conference::VideoSubscriptionConstraints>)
    nativeVideoSubscriptionConstraints;
@end
@interface OWTConferenceSubscribeOptions ()
- (std::shared_ptr<owt::conference::SubscribeOptions>)nativeSubscribeOptions;
@end
@interface OWTConferenceVideoSubscriptionUpdateConstraints ()
- (std::shared_ptr<owt::conference::VideoSubscriptionUpdateConstraints>)
    nativeVideoSubscriptionUpdateConstraints;
@end
@interface OWTConferenceSubscriptionUpdateOptions ()
- (std::shared_ptr<owt::conference::SubscriptionUpdateOptions>)
    nativeSubscriptionUpdateOptions;
@end
