// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_CONFERENCE_SUBSCRIPTION_H_
#define OWT_CONFERENCE_SUBSCRIPTION_H_
#include <vector>
#include <mutex>
#include "owt/base/commontypes.h"
#include "owt/base/mediaconstraints.h"
#include "owt/base/subscription.h"
#include "owt/base/connectionstats.h"
#include "owt/base/exception.h"
#include "owt/conference/streamupdateobserver.h"
#include "owt/conference/subscribeoptions.h"
namespace rtc {
  class TaskQueue;
}
namespace webrtc{
  class StatsReport;
}
namespace owt {
namespace conference {
class ConferenceClient;
using namespace owt::base;
class ConferenceSubscription : public ConferenceStreamUpdateObserver,
                               public std::enable_shared_from_this<ConferenceSubscription> {
  public:
    ConferenceSubscription(std::shared_ptr<ConferenceClient> client, const std::string& sub_id,
                           const std::string& stream_id);
    virtual ~ConferenceSubscription();
    /// Pause current publication's audio or/and video basing on |track_kind| provided.
    void Mute(TrackKind track_kind,
              std::function<void()> on_success,
              std::function<void(std::unique_ptr<Exception>)> on_failure);
    /// Pause current publication's audio or/and video basing on |track_kind| provided.
    void Unmute(TrackKind track_kind,
                std::function<void()> on_success,
                std::function<void(std::unique_ptr<Exception>)> on_failure);
    /// Get conneciton stats of current publication
    void GetStats(
        std::function<void(std::shared_ptr<ConnectionStats>)> on_success,
        std::function<void(std::unique_ptr<Exception>)> on_failure);
    void GetNativeStats(
        std::function<void(
            const std::vector<const webrtc::StatsReport*>& reports)> on_success,
        std::function<void(std::unique_ptr<Exception>)> on_failure);
    /// Stop current publication.
    void Stop();
    /// If the Subscription is stopped or not.
    bool Ended() { return ended_; }
    /// Get the subscription ID
    std::string Id() const { return id_; }
    /// Update the subscription with new encoding settings.
    void ApplyOptions(
        const SubscriptionUpdateOptions& options,
        std::function<void()> on_success,
        std::function<void(std::unique_ptr<Exception>)> on_failure);
    /// Add observer on the subscription
    void AddObserver(SubscriptionObserver& observer);
    /// Remove observer on the subscription.
    void RemoveObserver(SubscriptionObserver& observer);
  private:
    void OnStreamMuteOrUnmute(const std::string& stream_id, TrackKind track_kind, bool muted);
    void OnStreamRemoved(const std::string& stream_id);
    std::string id_;
    std::string stream_id_;
    bool ended_;
    mutable std::mutex observer_mutex_;
    std::vector<std::reference_wrapper<SubscriptionObserver>> observers_;
    std::weak_ptr<ConferenceClient>  conference_client_;   // Weak ref to associated conference client
    std::shared_ptr<rtc::TaskQueue> event_queue_;
};

} // namespace conference
} // namespace owt
#endif  // OWT_CONFERENCE_SUBSCRIPTION_H_
