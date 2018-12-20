// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OMS_P2P_PUBLICATION_H_
#define OMS_P2P_PUBLICATION_H_
#include <vector>
#include <mutex>
#include "oms/base/commontypes.h"
#include "oms/base/mediaconstraints.h"
#include "oms/base/publication.h"
namespace rtc {
class TaskQueue;
}
namespace oms {
namespace p2p {
using namespace oms::base;
class P2PClient;
class P2PPublication : public Publication {
 public:
  P2PPublication(std::shared_ptr<P2PClient> client, std::string target_id, std::shared_ptr<LocalStream> stream);
  virtual ~P2PPublication() {}
  /// Get connection stats of current publication
  void GetStats(
      std::function<void(std::shared_ptr<ConnectionStats>)> on_success,
      std::function<void(std::unique_ptr<Exception>)> on_failure) override;
  /// Stop current publication.
  void Stop() override;
  /// Pause current publication's audio or/and video basing on |track_kind| provided.
  /// Not supported in P2P yet.
  void Mute(TrackKind track_kind,
            std::function<void()> on_success,
            std::function<void(std::unique_ptr<Exception>)> on_failure) override {}
  /// Pause current publication's audio or/and video basing on |track_kind| provided.
  /// Not supported in P2P yet.
  void Unmute(TrackKind track_kind,
              std::function<void()> on_success,
              std::function<void(std::unique_ptr<Exception>)> on_failure) override {}
  /// Register an observer onto this p2p publication.
  void AddObserver(PublicationObserver& observer) override;
  /// Unregister an observer from this p2p publication.
  void RemoveObserver(PublicationObserver& observer) override;
 private:
  std::string target_id_;
  std::shared_ptr<LocalStream> local_stream_;
  std::weak_ptr<P2PClient> p2p_client_;   // Weak ref to associated p2p client
  std::shared_ptr<rtc::TaskQueue> event_queue_;
  mutable std::mutex observer_mutex_;
  std::vector<std::reference_wrapper<PublicationObserver>> observers_;
  bool ended_;
};
} // namespace p2p
} // namespace oms
#endif  // OMS_P2P_PUBLICATION_H_
