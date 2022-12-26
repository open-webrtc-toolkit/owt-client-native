// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_CONFERENCE_STREAMUPDATEOBSERVER_H
#define OWT_CONFERENCE_STREAMUPDATEOBSERVER_H
#include "owt/base/commontypes.h"
#ifdef OWT_ENABLE_QUIC
#include "owt/quic/web_transport_stream_interface.h"
#endif
namespace owt {
namespace conference {
/** @cond */
/// Observer provided to publication/subscription to report mute/unmute/error/removed event.
class OWT_EXPORT ConferenceStreamUpdateObserver {
public:
  /**
  @brief Triggers when audio or video status switched between active/inactive.
  */
  virtual void OnStreamMuteOrUnmute(const std::string& stream_id,
    owt::base::TrackKind track_kind, bool muted) {}
  virtual void OnStreamRemoved(const std::string& stream_id) {}
  virtual void OnStreamError(const std::string& error_msg){}
#ifdef OWT_ENABLE_QUIC
  virtual void OnIncomingStream(
      const std::string& session_id, owt::quic::WebTransportStreamInterface* stream) {}
#endif
};
/** @endcond */
}
}
#endif  // OWT_CONFERENCE_STREAMUPDATEOBSERVER_H
