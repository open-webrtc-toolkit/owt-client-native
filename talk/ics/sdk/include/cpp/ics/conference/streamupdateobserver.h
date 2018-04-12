/*
 * Intel License
 */

#ifndef ICS_CONFERENCE_STREAMUPDATEOBSERVER_H
#define ICS_CONFERENCE_STREAMUPDATEOBSERVER_H

#include "ics/base/commontypes.h"

namespace ics {
namespace conference {

/** @cond */
/// Observer provided to publication/subscription to report mute/unmute event.
class ConferenceStreamUpdateObserver {
public:
  /**
  @brief Triggers when audio or video status switched between active/inactive.
  */
  virtual void OnStreamMuteOrUnmute(const std::string& stream_id,
    ics::base::TrackKind track_kind, bool muted) {};
  virtual void OnStreamRemoved(const std::string& stream_id) {};
};
/** @endcond */
}
}

#endif  // ICS_CONFERENCE_STREAMUPDATEOBSERVER_H
