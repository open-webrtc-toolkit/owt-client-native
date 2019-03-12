// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_CONFERENCE_STREAMUPDATEOBSERVER_H
#define OWT_CONFERENCE_STREAMUPDATEOBSERVER_H
#include "owt/base/commontypes.h"
namespace owt {
namespace conference {
/** @cond */
/// Observer provided to publication/subscription to report mute/unmute event.
class ConferenceStreamUpdateObserver {
public:
  /**
  @brief Triggers when audio or video status switched between active/inactive.
  */
  virtual void OnStreamMuteOrUnmute(const std::string& stream_id,
    owt::base::TrackKind track_kind, bool muted) {};
  virtual void OnStreamRemoved(const std::string& stream_id) {};
};
/** @endcond */
}
}
#endif  // OWT_CONFERENCE_STREAMUPDATEOBSERVER_H
