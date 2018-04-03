/*
 * Copyright © 2017 Intel Corporation. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ICS_BASE_PUBLICATION_H_
#define ICS_BASE_PUBLICATION_H_

#include "ics/base/commontypes.h"
#include "ics/base/exception.h"
#include "ics/base/connectionstats.h"

namespace ics {
namespace base {

/// Observer that receives event from publication.
class PublicationObserver {
 public:
  /// Triggered when publication is ended.
  virtual void OnEnded() = 0;
  /// Triggered when audio and/or video is muted.
  virtual void OnMute(TrackKind track_kind) = 0;
  /// Triggered when audio and/or video is unmuted.
  virtual void OnUnmute(TrackKind track_kind) = 0;
};

class Publication {
 public:
  /// Pause current publication's audio or/and video basing on |track_kind| provided.
  virtual void Mute(TrackKind track_kind,
                    std::function<void()> on_success,
                    std::function<void(std::unique_ptr<Exception>)> on_failure) = 0;
  /// Pause current publication's audio or/and video basing on |track_kind| provided.
  virtual void Unmute(TrackKind track_kind,
                      std::function<void()> on_success,
                      std::function<void(std::unique_ptr<Exception>)> on_failure) = 0;
  /// Get conneciton stats of current publication
  virtual void GetStats(
      std::function<void(std::shared_ptr<ConnectionStats>)> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure) = 0;
  /// Stop current publication.
  virtual void Stop() = 0;
  /// Register an observer onto this publication.
  virtual void AddObserver(PublicationObserver& observer) = 0;
  /// Unregister an observer from this publication.
  virtual void RemoveObserver(PublicationObserver& observer) = 0;
};

} // namespace base
} // namespace ics

#endif  // ICS_BASE_PUBLICATION_H_
