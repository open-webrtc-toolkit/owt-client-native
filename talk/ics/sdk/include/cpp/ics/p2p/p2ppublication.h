/*
 * Copyright © 2018 Intel Corporation. All Rights Reserved.
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

#ifndef ICS_P2P_PUBLICATION_H_
#define ICS_P2P_PUBLICATION_H_

#include <vector>
#include <mutex>

#include "ics/base/commontypes.h"
#include "ics/base/mediaconstraints.h"
#include "ics/base/publication.h"
#include "ics/p2p/p2pclient.h"
#include "ics/p2p/p2pexception.h"

namespace rtc {
class TaskQueue;
}

namespace ics {
namespace p2p {

using namespace ics::base;

class P2PClient;

class P2PPublication {
 public:
  P2PPublication(std::shared_ptr<P2PClient> client, std::string target_id);
  virtual ~P2PPublication() {}

  /// Pause current publication's audio or/and video basing on |track_kind| provided.
  void Mute(TrackKind track_kind,
            std::function<void()> on_success,
            std::function<void(std::unique_ptr<P2PException>)> on_failure);
  /// Pause current publication's audio or/and video basing on |track_kind| provided.
  void UnMute(TrackKind track_kind,
              std::function<void()> on_success,
              std::function<void(std::unique_ptr<P2PException>)> on_failure);
  /// Get connection stats of current publication
  void GetStats(
    std::function<void(std::shared_ptr<ConnectionStats>)> on_success,
    std::function<void(std::unique_ptr<P2PException>)> on_failure);
  /// Stop current publication.
  void Stop(std::function<void()> on_success,
            std::function<void(std::unique_ptr<P2PException>)> on_failure);
 private:
  std::string target_id_;
  std::weak_ptr<P2PClient> p2p_client_;   // Weak ref to associated p2p client
  std::shared_ptr<rtc::TaskQueue> event_queue_;

  bool ended_;
};

} // namespace p2p
} // namespace ics

#endif  // ICS_P2P_PUBLICATION_H_
