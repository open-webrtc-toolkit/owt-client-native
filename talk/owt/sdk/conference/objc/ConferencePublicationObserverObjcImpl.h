// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_CONFERENCE_OBJC_CONFERENCEPUBLICATIONOBJCIMPL_H_
#define OWT_CONFERENCE_OBJC_CONFERENCEPUBLICATIONOBJCIMPL_H_
#include "talk/owt/sdk/include/cpp/owt/base/exception.h"
#include "talk/owt/sdk/include/cpp/owt/base/publication.h"
#import "talk/owt/sdk/include/objc/OWT/OWTConferencePublication.h"
namespace owt {
namespace conference {
class ConferencePublicationObserverObjcImpl : public owt::base::PublicationObserver {
 public:
  ConferencePublicationObserverObjcImpl(OWTConferencePublication* publication,
                                id<OWTConferencePublicationDelegate> delegate)
      : publication_(publication), delegate_(delegate) {}
  virtual ~ConferencePublicationObserverObjcImpl(){}
 protected:
  /// Triggered when publication is ended.
  virtual void OnEnded() override;
  /// Triggered when audio and/or video is muted.
  virtual void OnMute(owt::base::TrackKind track_kind) override;
  /// Triggered when audio and/or video is unmuted.
  virtual void OnUnmute(owt::base::TrackKind track_kind) override;
  /// Triggered on ice failure or server reported failure.
  virtual void OnError(std::unique_ptr<owt::base::Exception> error_info) override;
 private:
  __weak OWTConferencePublication* publication_;
  __weak id<OWTConferencePublicationDelegate> delegate_;
};
}  // namespace conference
}  // namespace owt
#endif
