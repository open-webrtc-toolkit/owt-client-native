// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_PUBLICATION_H_
#define OWT_BASE_PUBLICATION_H_
#include "owt/base/commontypes.h"
#include "owt/base/exception.h"
#include "owt/base/connectionstats.h"
namespace owt {
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
} // namespace owt
#endif  // OWT_BASE_PUBLICATION_H_
