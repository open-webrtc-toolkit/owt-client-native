//
//  Copyright (c) 2018 Intel Corporation. All rights reserved.
//
#include "talk/oms/sdk/conference/objc/ConferencePublicationObserverObjcImpl.h"
#include "webrtc/rtc_base/checks.h"
#import "talk/oms/sdk/base/objc/OMSMediaFormat+Private.h"
#import "talk/oms/sdk/conference/objc/OMSConferencePublication+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/PeerConnection/RTCLegacyStatsReport+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
#import <OMS/OMSErrors.h>
#import <OMS/OMSConferenceErrors.h>
@implementation OMSConferencePublication {
  std::shared_ptr<oms::conference::ConferencePublication> _nativePublication;
  std::unique_ptr<
      oms::conference::ConferencePublicationObserverObjcImpl,
      std::function<void(oms::conference::ConferencePublicationObserverObjcImpl*)>>
      _observer;
}
- (instancetype)initWithNativePublication:
    (std::shared_ptr<oms::conference::ConferencePublication>)nativePublication {
  self = [super init];
  _nativePublication = nativePublication;
  return self;
}
- (void)stop {
  _nativePublication->Stop();
}
- (void)statsWithOnSuccess:(void (^)(NSArray<RTCLegacyStatsReport*>*))onSuccess
                 onFailure:(nullable void (^)(NSError*))onFailure {
  RTC_CHECK(onSuccess);
  _nativePublication->GetNativeStats(
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
      [onFailure](std::unique_ptr<oms::base::Exception> e) {
        if (onFailure == nil)
          return;
        NSError* err = [[NSError alloc]
            initWithDomain:OMSErrorDomain
                      code:OMSConferenceErrorUnknown
                  userInfo:[[NSDictionary alloc]
                               initWithObjectsAndKeys:
                                   [NSString stringForStdString:e->Message()],
                                   NSLocalizedDescriptionKey, nil]];
        onFailure(err);
      });
}
- (void)mute:(OMSTrackKind)trackKind
    onSuccess:(nullable void (^)())onSuccess
    onFailure:(nullable void (^)(NSError*))onFailure {
  _nativePublication->Mute(
      [OMSTrackKindConverter cppTrackKindForObjcTrackKind:trackKind],
      [onSuccess]() {
        if (onSuccess)
          onSuccess();
      },
      [onFailure](std::unique_ptr<oms::base::Exception> e) {
        if (onFailure == nil)
          return;
        NSError* err = [[NSError alloc]
            initWithDomain:OMSErrorDomain
                      code:OMSConferenceErrorUnknown
                  userInfo:[[NSDictionary alloc]
                               initWithObjectsAndKeys:
                                   [NSString stringForStdString:e->Message()],
                                   NSLocalizedDescriptionKey, nil]];
        onFailure(err);
      });
}
- (void)unmute:(OMSTrackKind)trackKind
     onSuccess:(nullable void (^)())onSuccess
     onFailure:(nullable void (^)(NSError*))onFailure {
  _nativePublication->Unmute(
      [OMSTrackKindConverter cppTrackKindForObjcTrackKind:trackKind],
      [onSuccess]() {
        if (onSuccess)
          onSuccess();
      },
      [onFailure](std::unique_ptr<oms::base::Exception> e) {
        if (onFailure == nil)
          return;
        NSError* err = [[NSError alloc]
            initWithDomain:OMSErrorDomain
                      code:OMSConferenceErrorUnknown
                  userInfo:[[NSDictionary alloc]
                               initWithObjectsAndKeys:
                                   [NSString stringForStdString:e->Message()],
                                   NSLocalizedDescriptionKey, nil]];
        onFailure(err);
      });
}
- (NSString*)publicationId {
  return [NSString stringForStdString:_nativePublication->Id()];
}
-(void)setDelegate:(id<OMSConferencePublicationDelegate>)delegate{
  _observer = std::unique_ptr<
      oms::conference::ConferencePublicationObserverObjcImpl,
      std::function<void(oms::conference::ConferencePublicationObserverObjcImpl*)>>(
      new oms::conference::ConferencePublicationObserverObjcImpl(self, delegate),
      [&self](oms::conference::ConferencePublicationObserverObjcImpl* observer) {
        self->_nativePublication->RemoveObserver(*observer);
      });
  _nativePublication->AddObserver(*_observer.get());
  _delegate = delegate;
}
@end
