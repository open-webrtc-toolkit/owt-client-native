//
//  Copyright (c) 2018 Intel Corporation. All rights reserved.
//

#include "talk/ics/sdk/conference/objc/ConferencePublicationObserverObjcImpl.h"
#include "webrtc/rtc_base/checks.h"

#import "talk/ics/sdk/base/objc/ICSMediaFormat+Private.h"
#import "talk/ics/sdk/conference/objc/ICSConferencePublication+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/PeerConnection/RTCLegacyStatsReport+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
#import <ICS/ICSErrors.h>
#import <ICS/ICSConferenceErrors.h>

@implementation ICSConferencePublication {
  std::shared_ptr<ics::conference::ConferencePublication> _nativePublication;
  std::unique_ptr<
      ics::conference::ConferencePublicationObserverObjcImpl,
      std::function<void(ics::conference::ConferencePublicationObserverObjcImpl*)>>
      _observer;
}

- (instancetype)initWithNativePublication:
    (std::shared_ptr<ics::conference::ConferencePublication>)nativePublication {
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

- (void)mute:(ICSTrackKind)trackKind
    onSuccess:(nullable void (^)())onSuccess
    onFailure:(nullable void (^)(NSError*))onFailure {
  _nativePublication->Mute(
      [ICSTrackKindConverter cppTrackKindForObjcTrackKind:trackKind],
      [onSuccess]() {
        if (onSuccess)
          onSuccess();
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

- (void)unmute:(ICSTrackKind)trackKind
     onSuccess:(nullable void (^)())onSuccess
     onFailure:(nullable void (^)(NSError*))onFailure {
  _nativePublication->Unmute(
      [ICSTrackKindConverter cppTrackKindForObjcTrackKind:trackKind],
      [onSuccess]() {
        if (onSuccess)
          onSuccess();
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

-(void)setDelegate:(id<ICSConferencePublicationDelegate>)delegate{
  _observer = std::unique_ptr<
      ics::conference::ConferencePublicationObserverObjcImpl,
      std::function<void(ics::conference::ConferencePublicationObserverObjcImpl*)>>(
      new ics::conference::ConferencePublicationObserverObjcImpl(self, delegate),
      [&self](ics::conference::ConferencePublicationObserverObjcImpl* observer) {
        self->_nativePublication->RemoveObserver(*observer);
      });
  _nativePublication->AddObserver(*_observer.get());
  _delegate = delegate;
}

@end
