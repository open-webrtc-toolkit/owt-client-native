//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//
#import "talk/oms/sdk/include/cpp/oms/conference/conferencesubscription.h"
#import "talk/oms/sdk/include/cpp/oms/conference/subscribeoptions.h"
#import "talk/oms/sdk/include/objc/OMS/OMSConferenceSubscription.h"
@interface OMSConferenceSubscription ()
- (instancetype)initWithNativeSubscription:
    (std::shared_ptr<oms::conference::ConferenceSubscription>)
        nativeSubscription;
@end
@interface OMSConferenceAudioSubscriptionConstraints ()
- (std::shared_ptr<oms::conference::AudioSubscriptionConstraints>)
    nativeAudioSubscriptionConstraints;
@end
@interface OMSConferenceVideoSubscriptionConstraints ()
- (std::shared_ptr<oms::conference::VideoSubscriptionConstraints>)
    nativeVideoSubscriptionConstraints;
@end
@interface OMSConferenceSubscribeOptions ()
- (std::shared_ptr<oms::conference::SubscribeOptions>)nativeSubscribeOptions;
@end
@interface OMSConferenceVideoSubscriptionUpdateConstraints ()
- (std::shared_ptr<oms::conference::VideoSubscriptionUpdateConstraints>)
    nativeVideoSubscriptionUpdateConstraints;
@end
@interface OMSConferenceSubscriptionUpdateOptions ()
- (std::shared_ptr<oms::conference::SubscriptionUpdateOptions>)
    nativeSubscriptionUpdateOptions;
@end
