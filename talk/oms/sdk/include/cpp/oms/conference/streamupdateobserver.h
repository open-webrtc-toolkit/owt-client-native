/*
 * Intel License
 */
#ifndef OMS_CONFERENCE_STREAMUPDATEOBSERVER_H
#define OMS_CONFERENCE_STREAMUPDATEOBSERVER_H
#include "oms/base/commontypes.h"
namespace oms {
namespace conference {
/** @cond */
/// Observer provided to publication/subscription to report mute/unmute event.
class ConferenceStreamUpdateObserver {
public:
  /**
  @brief Triggers when audio or video status switched between active/inactive.
  */
  virtual void OnStreamMuteOrUnmute(const std::string& stream_id,
    oms::base::TrackKind track_kind, bool muted) {};
  virtual void OnStreamRemoved(const std::string& stream_id) {};
};
/** @endcond */
}
}
#endif  // OMS_CONFERENCE_STREAMUPDATEOBSERVER_H
