#ifndef ICS_CONFERENCE_OBJC_CONFERENCEPUBLICATIONOBJCIMPL_H_
#define ICS_CONFERENCE_OBJC_CONFERENCEPUBLICATIONOBJCIMPL_H_

#include "talk/ics/sdk/include/cpp/ics/base/publication.h"

#import "talk/ics/sdk/include/objc/ICS/ICSConferencePublication.h"

namespace ics {
namespace conference {
class ConferencePublicationObserverObjcImpl : public ics::base::PublicationObserver {
 public:
  ConferencePublicationObserverObjcImpl(ICSConferencePublication* publication,
                                id<ICSConferencePublicationDelegate> delegate)
      : publication_(publication), delegate_(delegate) {}

 protected:
  /// Triggered when publication is ended.
  virtual void OnEnded() override;
  /// Triggered when audio and/or video is muted.
  virtual void OnMute(ics::base::TrackKind track_kind) override;
  /// Triggered when audio and/or video is unmuted.
  virtual void OnUnmute(ics::base::TrackKind track_kind) override;

 private:
  ICSConferencePublication* publication_;
  id<ICSConferencePublicationDelegate> delegate_;
};
}  // namespace conference
}  // namespace ics

#endif
