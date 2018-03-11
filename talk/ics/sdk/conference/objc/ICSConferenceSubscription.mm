//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/conference/objc/ICSConferenceSubscription+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/PeerConnection/RTCLegacyStatsReport+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
#import <ICS/ICSErrors.h>
#import <ICS/ICSConferenceErrors.h>

#include "webrtc/rtc_base/checks.h"

@implementation ICSConferenceSubscription {
  std::shared_ptr<ics::conference::ConferenceSubscription> _nativeSubscription;
}

- (instancetype)initWithNativeSubscription:
    (std::shared_ptr<ics::conference::ConferenceSubscription>)
        nativeSubscription {
  self = [super init];
  _nativeSubscription = nativeSubscription;
  return self;
}

- (void)stop {
  _nativeSubscription->Stop(nullptr, nullptr);
}

- (void)statsWith:(void (^)(NSArray<RTCLegacyStatsReport*>*))onSuccess
        onFailure:(nullable void (^)(NSError*))onFailure {
  RTC_CHECK(onSuccess);
  _nativeSubscription->GetNativeStats(
      [onSuccess](const std::vector<const webrtc::StatsReport*>& reports) {
        NSMutableArray* stats =
            [NSMutableArray arrayWithCapacity:reports.size()];
        for (const auto* report : reports) {
          RTCLegacyStatsReport* statsReport =
              [[RTCLegacyStatsReport alloc] initWithNativeReport:*report];
          [stats addObject:statsReport];
        }
        onSuccess(stats);
      },
      [onFailure](std::unique_ptr<ics::base::Exception> e) {
        if (onFailure == nil)
          return;
        NSError* err = [[NSError alloc]
            initWithDomain:ICSErrorDomain
                      code:ICSConferenceErrorUnknown
                  userInfo:[[NSDictionary alloc]
                               initWithObjectsAndKeys:
                                   [NSString stringForStdString:e->Message()],
                                   NSLocalizedDescriptionKey, nil]];
        onFailure(err);
      });
}

@end

@implementation ICSConferenceSubscriptionOptions {
  CGSize _resolution;
}

- (instancetype)init {
  if ((self = [super init])) {
    self.resolution = CGSizeMake(0, 0);
  }
  return self;
}

- (CGSize)resolution {
  return _resolution;
}

- (void)setResolution:(CGSize)resolution {
  _resolution = resolution;
}

- (ics::conference::SubscriptionOptions)nativeSubscriptionOptions {
  using namespace ics::conference;
  ics::conference::SubscriptionOptions options;
  options.video.resolution.width = _resolution.width;
  options.video.resolution.height = _resolution.height;
  return options;
}

@end
