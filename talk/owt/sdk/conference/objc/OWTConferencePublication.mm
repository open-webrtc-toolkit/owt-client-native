// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "talk/owt/sdk/conference/objc/ConferencePublicationObserverObjcImpl.h"
#include "webrtc/rtc_base/checks.h"
#import "talk/owt/sdk/base/objc/OWTMediaFormat+Private.h"
#import "talk/owt/sdk/conference/objc/OWTConferencePublication+Private.h"
#import "webrtc/sdk/objc/api/peerconnection/RTCLegacyStatsReport+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
#import <OWT/OWTErrors.h>
#import <OWT/OWTConferenceErrors.h>
@implementation OWTConferencePublication {
  std::shared_ptr<owt::conference::ConferencePublication> _nativePublication;
  std::unique_ptr<
      owt::conference::ConferencePublicationObserverObjcImpl,
      std::function<void(owt::conference::ConferencePublicationObserverObjcImpl*)>>
      _observer;
}
- (instancetype)initWithNativePublication:
    (std::shared_ptr<owt::conference::ConferencePublication>)nativePublication {
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
      [onFailure](std::unique_ptr<owt::base::Exception> e) {
        if (onFailure == nil)
          return;
        NSError* err = [[NSError alloc]
            initWithDomain:OWTErrorDomain
                      code:OWTConferenceErrorUnknown
                  userInfo:[[NSDictionary alloc]
                               initWithObjectsAndKeys:
                                   [NSString stringForStdString:e->Message()],
                                   NSLocalizedDescriptionKey, nil]];
        onFailure(err);
      });
}
- (void)mute:(OWTTrackKind)trackKind
    onSuccess:(nullable void (^)())onSuccess
    onFailure:(nullable void (^)(NSError*))onFailure {
  _nativePublication->Mute(
      [OWTTrackKindConverter cppTrackKindForObjcTrackKind:trackKind],
      [onSuccess]() {
        if (onSuccess)
          onSuccess();
      },
      [onFailure](std::unique_ptr<owt::base::Exception> e) {
        if (onFailure == nil)
          return;
        NSError* err = [[NSError alloc]
            initWithDomain:OWTErrorDomain
                      code:OWTConferenceErrorUnknown
                  userInfo:[[NSDictionary alloc]
                               initWithObjectsAndKeys:
                                   [NSString stringForStdString:e->Message()],
                                   NSLocalizedDescriptionKey, nil]];
        onFailure(err);
      });
}
- (void)unmute:(OWTTrackKind)trackKind
     onSuccess:(nullable void (^)())onSuccess
     onFailure:(nullable void (^)(NSError*))onFailure {
  _nativePublication->Unmute(
      [OWTTrackKindConverter cppTrackKindForObjcTrackKind:trackKind],
      [onSuccess]() {
        if (onSuccess)
          onSuccess();
      },
      [onFailure](std::unique_ptr<owt::base::Exception> e) {
        if (onFailure == nil)
          return;
        NSError* err = [[NSError alloc]
            initWithDomain:OWTErrorDomain
                      code:OWTConferenceErrorUnknown
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
-(void)setDelegate:(id<OWTConferencePublicationDelegate>)delegate{
  if (delegate != nil) {
    __weak OWTConferencePublication *weakSelf = self;
    _observer = std::unique_ptr<
                owt::conference::ConferencePublicationObserverObjcImpl,
            std::function<void(owt::conference::ConferencePublicationObserverObjcImpl*)>>(
                    new owt::conference::ConferencePublicationObserverObjcImpl(self, delegate),
                            [=](owt::conference::ConferencePublicationObserverObjcImpl* observer) {
                              __strong OWTConferencePublication *strongSelf = weakSelf;
                              if (strongSelf) {
                                strongSelf->_nativePublication->RemoveObserver(*observer);
                              }
                              delete observer;
                            });
    _nativePublication->AddObserver(*_observer.get());
  } else {
    _observer.reset();
  }
  _delegate = delegate;
}
@end
