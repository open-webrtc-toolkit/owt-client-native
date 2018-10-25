#ifndef OMS_CONFERENCE_OBJC_CONFERENCEPUBLICATIONOBJCIMPL_H_
#define OMS_CONFERENCE_OBJC_CONFERENCEPUBLICATIONOBJCIMPL_H_

#include "talk/oms/sdk/include/cpp/oms/base/publication.h"

#import "talk/oms/sdk/include/objc/OMS/OMSConferencePublication.h"

namespace oms {
namespace conference {
class ConferencePublicationObserverObjcImpl : public oms::base::PublicationObserver {
 public:
  ConferencePublicationObserverObjcImpl(OMSConferencePublication* publication,
                                id<OMSConferencePublicationDelegate> delegate)
      : publication_(publication), delegate_(delegate) {}

 protected:
  /// Triggered when publication is ended.
  virtual void OnEnded() override;
  /// Triggered when audio and/or video is muted.
  virtual void OnMute(oms::base::TrackKind track_kind) override;
  /// Triggered when audio and/or video is unmuted.
  virtual void OnUnmute(oms::base::TrackKind track_kind) override;

 private:
  OMSConferencePublication* publication_;
  id<OMSConferencePublicationDelegate> delegate_;
};
}  // namespace conference
}  // namespace oms

#endif
