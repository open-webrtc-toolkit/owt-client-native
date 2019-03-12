// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_CONFERENCE_PUBLICATION_H_
#define OWT_CONFERENCE_PUBLICATION_H_
#include <vector>
#include <mutex>
#include "owt/base/commontypes.h"
#include "owt/base/mediaconstraints.h"
#include "owt/base/publication.h"
#include "owt/conference/streamupdateobserver.h"
namespace rtc {
  class TaskQueue;
}
namespace webrtc{
  class StatsReport;
}
namespace owt {
namespace base {
struct ConnectionStats;
}
namespace conference {
class ConferenceClient;
using namespace owt::base;
class ConferencePublication : public Publication, public ConferenceStreamUpdateObserver {
  public:
    ConferencePublication(std::shared_ptr<ConferenceClient> client, const std::string& pub_id,
                          const std::string& stream_id);
    virtual ~ConferencePublication();
    /// Return the ID of the publication.
    std::string Id() const { return id_; }
    /// Pause current publication's audio or/and video basing on |track_kind| provided.
    void Mute(TrackKind track_kind,
              std::function<void()> on_success,
              std::function<void(std::unique_ptr<Exception>)> on_failure) override;
    /// Pause current publication's audio or/and video basing on |track_kind| provided.
    void Unmute(TrackKind track_kind,
                std::function<void()> on_success,
                std::function<void(std::unique_ptr<Exception>)> on_failure) override;
    /// Get conneciton stats of current publication
    void GetStats(
        std::function<void(std::shared_ptr<ConnectionStats>)> on_success,
        std::function<void(std::unique_ptr<Exception>)> on_failure) override;
    void GetNativeStats(
        std::function<void(
            const std::vector<const webrtc::StatsReport*>& reports)> on_success,
        std::function<void(std::unique_ptr<Exception>)> on_failure);
    /// Stop current publication.
    void Stop() override;
    /// Check if the publication is stopped or not
    bool Ended() const { return ended_; }
    /// Register an observer onto this conference publication.
    void AddObserver(PublicationObserver& observer) override;
    /// Unregister an observer from this conference publication.
    void RemoveObserver(PublicationObserver& observer) override;
  private:
    void OnStreamMuteOrUnmute(const std::string& stream_id, TrackKind track_kind, bool muted) override;
     std::string id_;
     std::string stream_id_;
     bool ended_;
     mutable std::mutex observer_mutex_;
     std::vector<std::reference_wrapper<PublicationObserver>> observers_;
     std::weak_ptr<ConferenceClient> conference_client_;   // Weak ref to associated conference client
     std::shared_ptr<rtc::TaskQueue> event_queue_;
};
} // namespace conference
} // namespace owt
#endif  // OWT_CONFERENCE_PUBLICATION_H_
